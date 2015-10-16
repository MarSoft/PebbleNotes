import json
import re
from contextlib import contextmanager
from datetime import datetime

import httpretty as _httpretty
import pytest
from httpretty import DELETE, GET, POST, PUT
from requests import HTTPError

from pypebbleapi import Timeline
from pypebbleapi.schemas import DocumentError
from pypebbleapi.timeline import validate_pin

FAKE_API_ROOT = 'http://timeline_api'
FAKE_API_KEY = 'FAKE_API_KEY'

PEBBLE_API_ROOT = 'https://timeline-api.getpebble.com'

fake_pin = {
    'id': '1234',
    'time': datetime.now().isoformat(),
}


def urlize(partial_url):
    return re.compile(re.escape(FAKE_API_ROOT) + partial_url)


@contextmanager
def server(status_code):
    _httpretty.enable()
    _httpretty.register_uri(
        PUT,
        urlize(r"/v1/shared/pins/(.*)"),
        body=b'OK',
        content_type="application/json",
        status=status_code)
    _httpretty.register_uri(
        PUT,
        urlize(r"/v1/user/pins/(.*)"),
        body=b'OK',
        content_type="application/json",
        status=status_code)
    yield

    _httpretty.disable()
    _httpretty.reset()


@pytest.fixture
def httpretty(request):
    _httpretty.enable()

    def fin():
        _httpretty.disable()
        _httpretty.reset()

    request.addfinalizer(fin)
    return _httpretty


@pytest.fixture
def timeline():
    return Timeline(api_root=FAKE_API_ROOT, api_key=FAKE_API_KEY)


def test_validate_good_pin():
    good_pin = {
        "id": "meeting-453923",
        "time": 'asdasdasd',
    }

    validate_pin(good_pin)


def test_validate_bad_pin():
    bad_pin = {
        'id': 123
    }

    with pytest.raises(DocumentError):
        validate_pin(bad_pin)


def test_can_create_with_no_opts():
    t = Timeline()

    assert t


def test_sets_default_api_root():
    t = Timeline()

    assert t._api_root == PEBBLE_API_ROOT


def test_can_set_api_key():
    t = Timeline(api_key='TEST_KEY')

    assert t._api_key == 'TEST_KEY'


def test_can_set_api_root():
    t = Timeline(api_root=FAKE_API_ROOT)

    assert t._api_root == FAKE_API_ROOT


def test_send_shared_pin(timeline, httpretty):
    httpretty.register_uri(PUT, urlize(r"/v1/shared/pins/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.send_shared_pin(['a', 'b'], fake_pin)

    httpretty.register_uri(PUT, urlize(r"/v1/shared/pins/(.*)"),
                           body=b'BAD', status=400, content_type="application/json")

    with pytest.raises(HTTPError):
        timeline.send_shared_pin(['a', 'b'], fake_pin)


def test_delete_shared_pin(timeline, httpretty):
    httpretty.register_uri(DELETE, urlize(r"/v1/shared/pins/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.delete_shared_pin('test')

    httpretty.register_uri(DELETE, urlize(r"/v1/shared/pins/(.*)"),
                           body=b'BAD', status=400, content_type="application/json")

    with pytest.raises(HTTPError):
        timeline.delete_shared_pin('test')


def test_send_user_pin(timeline, httpretty):
    httpretty.register_uri(PUT, urlize(r"/v1/user/pins/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.send_user_pin(user_token='3323', pin=fake_pin)

    httpretty.register_uri(PUT, urlize(r"/v1/user/pins/(.*)"),
                           body=b'OK', status=400, content_type="application/json")

    with pytest.raises(HTTPError):
        timeline.send_user_pin(user_token='3323', pin=fake_pin)


def test_delete_user_pin(timeline, httpretty):
    httpretty.register_uri(DELETE, urlize(r"/v1/user/pins/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.delete_user_pin(user_token='3323', pin_id='test')

    httpretty.register_uri(DELETE, urlize(r"/v1/user/pins/(.*)"),
                           body=b'OK', status=400, content_type="application/json")

    with pytest.raises(HTTPError):
        timeline.delete_user_pin(user_token='3323', pin_id='test')


def test_list_user_subscriptions(timeline, httpretty):
    response = {'topics': ['a', 'b']}
    httpretty.register_uri(GET, urlize(r"/v1/user/subscriptions"),
                           body=json.dumps(response), status=200, content_type="application/json")

    assert timeline.list_subscriptions(user_token='testuser') == ['a', 'b']

    httpretty.register_uri(GET, urlize(r"/v1/user/subscriptions"),
                           body='{}', status=400, content_type="application/json")

    with pytest.raises(HTTPError):
        timeline.list_subscriptions(user_token='testuser')


def test_subscribe(timeline, httpretty):
    httpretty.register_uri(POST, urlize(r"/v1/user/subscriptions/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.subscribe('testuser', 'testtopic')

    assert httpretty.last_request().method == 'POST'


def test_unsubscribe(timeline, httpretty):
    httpretty.register_uri(DELETE, urlize(r"/v1/user/subscriptions/(.*)"),
                           body=b'OK', status=200, content_type="application/json")

    timeline.unsubscribe('testuser', 'testtopic')

    assert httpretty.last_request().method == 'DELETE'
