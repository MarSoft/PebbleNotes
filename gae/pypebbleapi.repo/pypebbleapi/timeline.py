"""
This module contains the main Timeline class.
"""
import requests
from cerberus import Validator as _Validator

from pypebbleapi import __version__, schemas

PEBBLE_API_ROOT = 'https://timeline-api.getpebble.com'
""" Default Pebble API root URL """


PEBBLE_CODES = {
    400: 'The pin object submitted was invalid.',
    403: 'The API key submitted was invalid.',
    410: 'The user token submitted was invalid or does not exist.',
    429: 'Server is sending updates too quickly.',
    503: 'Could not save pin due to a temporary server error.',
}
""" Mapping between error status codes and reason. """


def _raise_for_status(response):
    status = response.status_code
    if 400 <= response.status_code < 500:
        try:
            body = response.json()
            response.reason = body['errorMessage']
            return response.raise_for_status()
        except (KeyError, ValueError):
            pass

        try:
            response.reason = PEBBLE_CODES[status]
            return response.raise_for_status()
        except KeyError:
            pass

    response.raise_for_status()


def _request(method, url, api_key=None, user_token=None, topics_list=None,
        json=None, user_agent=None):
    headers = {}
    if api_key:
        headers['X-API-Key'] = api_key
    if user_token:
        headers['X-User-Token'] = user_token
    if topics_list:
        headers['X-PIN-Topics'] = ','.join(t for t in topics_list)
    if user_agent:
        headers['User-Agent'] = user_agent

    return requests.request(method, url, headers=headers, json=json)


def validate_pin(pin):
    """ Validate the given pin against the schema.

    :param dict pin: The pin to validate:
    :raises pypebbleapi.schemas.DocumentError: If the pin is not valid.
    """
    v = _Validator(schemas.pin)
    if v.validate(pin):
        return
    else:
        raise schemas.DocumentError(errors=v.errors)


class Timeline(object):
    """
    Timeline class used to send or delete pins and to access users' subscriptions.

    :param str api_key: Your application api key (either for production or sandbox)
    :param str api_root: Base URL to connect to. You should probably leave it to the default.
    """
    user_agent = '''pypebbleapi/{}'''.format(__version__)

    @property
    def api_key(self):
        return self._api_key

    @property
    def api_root(self):
        return self._api_root

    def __init__(self, api_key=None, api_root=PEBBLE_API_ROOT):
        self._api_key = api_key

        self._api_root = api_root

    def url_v1(self, partial_url=None):
        return '{}/v1{}'.format(self.api_root, partial_url)

    def send_shared_pin(self, topics, pin, skip_validation=False):
        """
        Send a shared pin for the given topics.

        :param list topics: The list of topics.
        :param dict pin: The pin.
        :param bool skip_validation: Whether to skip the validation.
        :raises pypebbleapi.schemas.DocumentError: If the validation process failed.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        if not self.api_key:
            raise ValueError("You need to specify an api_key.")
        if not skip_validation:
            validate_pin(pin)

        response = _request('PUT',
            url=self.url_v1('/shared/pins/' + pin['id']),
            user_agent=self.user_agent,
            api_key=self.api_key,
            topics_list=topics,
            json=pin,
        )
        _raise_for_status(response)

    def delete_shared_pin(self, pin_id):
        """
        Delete a shared pin.

        :param str pin_id: The id of the pin to delete.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        if not self.api_key:
            raise ValueError("You need to specify an api_key.")

        response = _request('DELETE',
            url=self.url_v1('/shared/pins/' + pin_id),
            user_agent=self.user_agent,
            api_key=self.api_key,
        )
        _raise_for_status(response)

    def send_user_pin(self, user_token, pin, skip_validation=False):
        """
        Send a user pin.

        :param str user_token: The token of the user.
        :param dict pin: The pin.
        :param bool skip_validation: Whether to skip the validation.
        :raises pypebbleapi.schemas.DocumentError: If the validation process failed.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        if not skip_validation:
            validate_pin(pin)

        response = _request('PUT',
            url=self.url_v1('/user/pins/' + pin['id']),
            user_agent=self.user_agent,
            user_token=user_token,
            json=pin,
        )
        _raise_for_status(response)

    def delete_user_pin(self, user_token, pin_id):
        """
        Delete a user pin.

        :param str user_token: The token of the user.
        :param str pin_id: The id of the pin to delete.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """

        response = _request('DELETE',
            url=self.url_v1('/user/pins/' + pin_id),
            user_agent=self.user_agent,
            user_token=user_token,
        )
        _raise_for_status(response)

    def subscribe(self, user_token, topic):
        """
        Subscribe a user to the given topic.

        :param str user_token: The token of the user.
        :param str topic: The topic.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        response = _request('POST',
            url=self.url_v1('/user/subscriptions/' + topic),
            user_agent=self.user_agent,
            user_token=user_token,
        )
        _raise_for_status(response)

    def unsubscribe(self, user_token, topic):
        """
        Unsubscribe a user from the given topic.

        :param str user_token: The token of the user.
        :param str topic: The topic.
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        response = _request('DELETE',
            url=self.url_v1('/user/subscriptions/' + topic),
            user_agent=self.user_agent,
            user_token=user_token,
        )
        _raise_for_status(response)

    def list_subscriptions(self, user_token):
        """
        Get the list of the topics which a user is subscribed to.

        :param str user_token: The token of the user.
        :return: The list of the topics.
        :rtype: list
        :raises `requests.exceptions.HTTPError`: If an HTTP error occurred.
        """
        response = _request('GET',
            url=self.url_v1('/user/subscriptions'),
            user_agent=self.user_agent,
            user_token=user_token,
        )
        _raise_for_status(response)

        return response.json()['topics']
