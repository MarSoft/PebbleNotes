import pytest
from cerberus import Validator

from pypebbleapi import schemas

fake_layout = {
    'type': 'genericPin',
    'title': 'Pin Title',
    'tinyIcon': schemas.icon_name_to_path('NOTIFICATION_FLAG'),
}
fake_datetime = "2015-08-30T14:01:46.183869"


@pytest.fixture
def pin_validator():
    return Validator(schemas.pin)


action_test_data = [
    ({
         'title': 5,
         'type': 'openWatchApp',
         'launchCode': 0,
     }, ['title']),
    ({
         'title': 'Action',
         'type': 5,
         'launchCode': 0,
     }, ['type']),
    ({
         'title': 'Action',
         'type': 'openWatchApp',
         'launchCode': '0',
     }, ['launchCode']),
    ({
         'title': 'Action',
         'type': 'openWatchApp',
         'launchCode': 0,
     }, [])
]


@pytest.mark.parametrize(['data', 'error_keys'], action_test_data)
def test_action(data, error_keys):
    action_validator = Validator(schemas.action)
    action_validator(data)

    assert set(error_keys) == set(action_validator.errors.keys())


notification_test_data = [
    ({
         'time': fake_datetime,
         'layout': fake_layout,
     }, []),
    ({
         'time': fake_datetime,
     }, ['layout'])
]


@pytest.mark.parametrize(['data', 'error_keys'], notification_test_data)
def test_notification(data, error_keys):
    notification_validator = Validator(schemas.notification)
    notification_validator(data)

    assert set(error_keys) == set(notification_validator.errors.keys())


reminder_test_data = [
    ({
         'time': fake_datetime,
         'layout': fake_layout,
     }, []),
]


@pytest.mark.parametrize(['data', 'error_keys'], reminder_test_data)
def test_reminder(data, error_keys):
    reminder_validator = Validator(schemas.reminder)
    reminder_validator(data)

    assert set(error_keys) == set(reminder_validator.errors.keys())

common_layout_test_data = [
    ({
         'type': 5,
     }, ['type']),
    ({}, ['type']),
    ({
        'type': 'NOT A LAYOUT'
    }, ['type']),
]


@pytest.mark.parametrize(['data', 'error_keys'], common_layout_test_data)
def test_common_layouts(data, error_keys):
    for l in schemas.layouts:
        layout_validator = Validator(l)
        layout_validator(data)

        assert set(error_keys) <= set(layout_validator.errors.keys())

# TODO: more generic layout tests
generic_layout_test_data = [
    ({
        'type': 'genericPin',
        'title': 5,
    }, ['title', 'tinyIcon']),
]

@pytest.mark.parametrize(['data', 'error_keys'], generic_layout_test_data)
def test_generic_layout(data, error_keys):
    generic_layout_validator = Validator(schemas.generic_layout)
    generic_layout_validator(data)

    assert set(error_keys) == set(generic_layout_validator.errors.keys())

# TODO: many more tests :)


def test_simple_document():
    v = Validator(schemas.pin)
    assert v
    document = {
        'id': '3',
        'time': "2015-08-27T23:57:11.136Z",
        'duration': 4,
    }
    assert v.validate(document)


def test_minimal_pin(pin_validator):
    document = {
        "id": "example-pin-generic-1",
        "time": "2015-03-19T18:00:00Z",
        "layout": {
            "type": "genericPin",
            "title": "News at 6 o'clock",
            "tinyIcon": "system://images/NOTIFICATION_FLAG"
        }
    }

    pin_validator.validate(document)
    assert pin_validator.errors == {}


def test_complete_pin(pin_validator):
    document = {
        "id": "meeting-453923",
        "time": "2015-03-19T15:00:00Z",
        "duration": 60,
        "createNotification": {
            "layout": {
                "type": "genericNotification",
                "title": "New Item",
                "tinyIcon": "system://images/NOTIFICATION_FLAG",
                "body": "A new appointment has been added to your calendar at 4pm."
            }
        },
        "updateNotification": {
            "time": "2015-03-19T16:00:00Z",
            "layout": {
                "type": "genericNotification",
                "tinyIcon": "system://images/NOTIFICATION_FLAG",
                "title": "Reminder",
                "body": "The meeting has been rescheduled to 4pm."
            }
        },
        "layout": {
            "title": "Client Meeting",
            "type": "genericPin",
            "tinyIcon": "system://images/TIMELINE_CALENDAR",
            "body": "Meeting in Kepler at 4:00pm. Topic: discuss pizza toppings for party."
        },
        "reminders": [
            {
                "time": "2015-03-19T14:45:00Z",
                "layout": {
                    "type": "genericReminder",
                    "tinyIcon": "system://images/TIMELINE_CALENDAR",
                    "title": "Meeting in 15 minutes"
                }
            },
            {
                "time": "2015-03-19T14:55:00Z",
                "layout": {
                    "type": "genericReminder",
                    "tinyIcon": "system://images/TIMELINE_CALENDAR",
                    "title": "Meeting in 5 minutes"
                }
            }
        ],
        "actions": [
            {
                "title": "View Schedule",
                "type": "openWatchApp",
                "launchCode": 15
            },
            {
                "title": "Show Directions",
                "type": "openWatchApp",
                "launchCode": 22
            }
        ]
    }

    pin_validator.validate(document)
    assert pin_validator.errors == {}


def test_pin_generic_layout(pin_validator):
    document = {
        "id": "pin-generic-1",
        "time": "2015-03-18T15:45:00Z",
        "layout": {
            "type": "genericPin",
            "title": "This is a genericPin!",
            "tinyIcon": "system://images/NOTIFICATION_FLAG",
            "primaryColor": "#FFFFFF",
            "secondaryColor": "#666666",
            "backgroundColor": "#222222"
        }
    }

    pin_validator.validate(document)
    assert pin_validator.errors == {}


def test_pin_calendar_layout(pin_validator):
    document = {
        "id": "pin-calendar-1",
        "time": "2015-03-18T15:45:00Z",
        "duration": 60,
        "layout": {
            "type": "calendarPin",
            "title": "Pin Layout Meeting",
            "locationName": "Conf Room 1",
            "body": "Discuss layout types with Design Team."
        }
    }

    pin_validator.validate(document)
    assert pin_validator.errors == {}
