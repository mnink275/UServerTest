#include <userver/clients/dns/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include <userver/utest/using_namespace_userver.hpp>

#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utils/daemon_run.hpp>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "hello.hpp"

namespace ink {

KeyValue::KeyValue(const components::ComponentConfig& config,
                   const components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<components::Postgres>("key-value-database")
              .GetCluster()) {
  constexpr auto kCreateTable = R"~(
    CREATE TABLE couriers(
      courier_id INTEGER PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
      courier_type TEXT,
      area INTEGER,
      begin_time VARCHAR,
      end_time VARCHAR)
    )~";

  using storages::postgres::ClusterHostType;
  pg_cluster_->Execute(ClusterHostType::kMaster, kCreateTable);
}

std::string KeyValue::HandleRequestThrow(
    const server::http::HttpRequest& request,
    server::request::RequestContext&) const {
  const auto& key = request.GetArg("courier_id");
  if (key.empty()) {
    throw server::handlers::ClientError(
        server::handlers::ExternalBody{"No 'courier_id' query argument"});
  }

  switch (request.GetMethod()) {
    case server::http::HttpMethod::kGet:
      return GetValue(key, request);
    case server::http::HttpMethod::kPost:
      return PostValue(key, request);
    case server::http::HttpMethod::kDelete:
      return DeleteValue(key);
    default:
      throw server::handlers::ClientError(server::handlers::ExternalBody{
          fmt::format("Unsupported method {}", request.GetMethod())});
  }
}

const storages::postgres::Query kSelectValue{
    "SELECT ROW(courier_type, area, begin_time, end_time) "
    "FROM couriers WHERE courier_id=$1",
    storages::postgres::Query::Name{"sample_select_value"},
};

std::string KeyValue::GetValue(std::string_view key,
                               const server::http::HttpRequest& request) const {
  storages::postgres::ResultSet res = pg_cluster_->Execute(
      storages::postgres::ClusterHostType::kSlave, kSelectValue, std::stoi(key.data()));
  if (res.IsEmpty()) {
    request.SetResponseStatus(server::http::HttpStatus::kNotFound);
    return {};
  }

  using TupleType = std::tuple<std::string, int, std::string, std::string>;
  auto tuple = res.AsSingleRow<TupleType>(storages::postgres::kFieldTag);
  return std::get<0>(tuple);
}

const storages::postgres::Query kInsertValue{
    "INSERT INTO couriers (courier_type, area, begin_time, end_time) "
    "VALUES($1, $2, $3, $4) "
    "ON CONFLICT DO NOTHING",
    storages::postgres::Query::Name{"sample_insert_value"},
};

std::string KeyValue::PostValue(
    std::string_view key, const server::http::HttpRequest& request) const {
  const auto& courier_type = request.GetArg("courier_type");
  const auto& area = request.GetArg("area");
  const auto& begin_time = request.GetArg("begin_time");
  const auto& end_time = request.GetArg("end_time");

  storages::postgres::Transaction transaction =
      pg_cluster_->Begin("sample_transaction_insert_key_value",
                         storages::postgres::ClusterHostType::kMaster, {});

  auto res = transaction.Execute(kInsertValue, courier_type,
                                 std::stoi(area.data()), begin_time, end_time);
  if (res.RowsAffected()) {
    transaction.Commit();
    request.SetResponseStatus(server::http::HttpStatus::kCreated);
    return std::string{};
  }

  res = transaction.Execute(kSelectValue, key);
  transaction.Rollback();

  auto result = res.AsSingleRow<std::string>();
  // if (result != value) {
  //   request.SetResponseStatus(server::http::HttpStatus::kConflict);
  // }

  return res.AsSingleRow<std::string>();
}

std::string KeyValue::DeleteValue(std::string_view key) const {
  auto res =
      pg_cluster_->Execute(storages::postgres::ClusterHostType::kMaster,
                           "DELETE FROM key_value_table WHERE key=$1", key);
  return std::to_string(res.RowsAffected());
}

}  // namespace ink