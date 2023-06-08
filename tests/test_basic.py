import pytest

from testsuite.databases import pgsql


# Start the tests via `make test-debug` or `make test-release`


# /// [Functional test]
async def test_postgres(service_client):
    response = await service_client.delete('/v1/key-value?key=hello')
    assert response.status == 200
 
    response = await service_client.post('/v1/key-value?key=hello&value=world')
    assert response.status == 201
    assert response.content == b'world'
 
    response = await service_client.get('/v1/key-value?key=hello')
    assert response.status == 200
    assert response.content == b'world'
 
    response = await service_client.delete('/v1/key-value?key=hello')
    assert response.status == 200
 
    response = await service_client.post('/v1/key-value?key=hello&value=there')
    assert response.status == 201
    assert response.content == b'there'
    # /// [Functional test]
 
    response = await service_client.get('/v1/key-value?key=hello')
    assert response.status == 200
    assert response.content == b'there'
 
    response = await service_client.post('/v1/key-value?key=hello&value=again')
    assert response.status == 409
    assert response.content == b'there'
 
    response = await service_client.get('/v1/key-value?key=missing')
    assert response.status == 404


# async def test_first_time_users(service_client):
#     response = await service_client.post(
#         '/v1/hello',
#         params={'name': 'userver'},
#     )
#     assert response.status == 200
#     assert response.text == 'Hello, userver!\n'


# async def test_db_updates(service_client):
#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hello, World!\n'

#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hi again, World!\n'

#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hi again, World!\n'


# @pytest.mark.pgsql('db_1', files=['initial_data.sql'])
# async def test_db_initial_data(service_client):
#     response = await service_client.post(
#         '/v1/hello',
#         params={'name': 'user-from-initial_data.sql'},
#     )
#     assert response.status == 200
#     assert response.text == 'Hi again, user-from-initial_data.sql!\n'
