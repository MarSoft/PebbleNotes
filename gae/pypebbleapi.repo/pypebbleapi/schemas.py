"""
This module contains the schemas used to validate the pin.
"""

from copy import deepcopy
from pprint import pformat


class DocumentError(ValueError):
    """
    Exception that is raised when a document fails the validation.
    """

    def __init__(self, errors):
        """
        Initialize DocumentError with an `errors` object.
        """
        self.errors = errors
        message = "One or more errors occurred when validating the document:"
        message += pformat(errors)
        self.errors = errors
        super(DocumentError, self).__init__(message)

layout_types = {
    'genericPin',
    'calendarPin',
    'genericReminder',
    'genericNotification',
    'commNotification',
    'weatherPin',
    'sportsPin',
}
""" Types of layouts """


def icon_name_to_path(name):
    """
    Convert an icon name to the corresponding URI.

    :param str name: The name of the icon.
    :return: The corresponding URI.
    :rtype: str
    """
    return 'system://images/' + name

icons = {icon_name_to_path(icon) for icon in {
    # Notifications
    'NOTIFICATION_GENERIC',
    'NOTIFICATION_REMINDER',
    'NOTIFICATION_FLAG',
    'NOTIFICATION_FACEBOOK_MESSENGER',
    'NOTIFICATION_WHATSAPP',
    'NOTIFICATION_GMAIL',
    'NOTIFICATION_FACEBOOK',
    'NOTIFICATION_GOOGLE_HANGOUTS',
    'NOTIFICATION_TELEGRAM',
    'NOTIFICATION_TWITTER',
    'NOTIFICATION_GOOGLE_INBOX',
    'NOTIFICATION_MAILBOX',
    'NOTIFICATION_OUTLOOK',
    'NOTIFICATION_INSTAGRAM',
    'NOTIFICATION_BLACKBERRY_MESSENGER',
    'NOTIFICATION_LINE',
    'NOTIFICATION_SNAPCHAT',
    'NOTIFICATION_WECHAT',
    'NOTIFICATION_VIBER',
    'NOTIFICATION_SKYPE',
    'NOTIFICATION_YAHOO_MAIL',
    # Generic
    'GENERIC_EMAIL',
    'GENERIC_SMS',
    'GENERIC_WARNING',
    'GENERIC_CONFIRMATION',
    'GENERIC_QUESTION',
    # Weather
    'PARTLY_CLOUDY',
    'CLOUDY_DAY',
    'LIGHT_SNOW',
    'LIGHT_RAIN',
    'HEAVY_RAIN',
    'HEAVY_SNOW',
    'TIMELINE_WEATHER',
    'TIMELINE_SUN',
    'RAINING_AND_SNOWING',
    # Timeline
    'TIMELINE_MISSED_CALL',
    'TIMELINE_CALENDAR',
    'TIMELINE_SPORTS',
    # Sports
    'TIMELINE_BASEBALL',
    'AMERICAN_FOOTBALL',
    'BASKETBALL',
    'CRICKET_GAME',
    'SOCCER_GAME',
    'HOCKEY_GAME',
    # Action Results
    'RESULT_DISMISSED',
    'RESULT_DELETED',
    'RESULT_MUTE',
    'RESULT_SENT',
    'RESULT_FAILED',
    # Miscellaneous
    'STOCKS_EVENT',
    'MUSIC_EVENT',
    'BIRTHDAY_EVENT',
    'PAY_BILL',
    'HOTEL_RESERVATION',
    'TIDE_IS_HIGH',
    'NEWS_EVENT',
    'SCHEDULED_EVENT',
    'DURING_PHONE_CALL',
    'CHECK_INTERNET_CONNECTION',
    'MOVIE_EVENT',
    'GLUCOSE_MONITOR',
    'ALARM_CLOCK',
    'CAR_RENTAL',
    'DINNER_RESERVATION',
    'RADIO_SHOW',
    'AUDIO_CASSETTE',
    'SCHEDULED_FLIGHT',
    'NO_EVENTS',
    'REACHED_FITNESS_GOAL',
    'DAY_SEPARATOR',
    'WATCH_DISCONNECTED',
    'TV_SHOW',
    # Deprecated
    'BASEBALL',
    'CHAT',
    'TAPE',
    'FOOTBALL',
    'MAIL',
    'BULB',
    'CALENDAR',
    'SUN',
    'PIN',
    'BATT_FULL',
    'BATT_EMPTY',
    'ALARM',
}}
""" Available icons list """

basic_layout = {
    'type': {'required': True, 'type': 'string', 'allowed': layout_types},
    'type': {'required': True, 'type': 'string'},
    'title': {'type': 'string'},
    'subtitle': {'type': 'string'},
    'body': {'type': 'string'},
    'tinyIcon': {'type': 'string', 'allowed': icons},
    'smallIcon': {'type': 'string', 'allowed': icons},
    'largeIcon': {'type': 'string', 'allowed': icons},
}

extended_layout = deepcopy(basic_layout)
extended_layout.update({
    'primaryColor': {'type': 'string'},  # TODO: constraints
    'secondaryColor': {'type': 'string'},  # TODO: constraints
    'backgroundColor': {'type': 'string'},  # TODO: constraints
    'headings': {'type': 'list', 'schema': {'type': 'string'}},
    'paragraphs': {'type': 'list', 'schema': {'type': 'string'}},  # TODO: constraints
    'lastUpdated': {'type': 'string'},  # TODO: datetime
})

generic_layout = deepcopy(extended_layout)
generic_layout['type']['allowed'] = ['genericPin']
generic_layout['title']['required'] = True
generic_layout['tinyIcon']['required'] = True



calendar_layout = deepcopy(extended_layout)
calendar_layout['type']['allowed'] = ['calendarPin']
calendar_layout['title']['required'] = True
calendar_layout.update({
    'locationName': {'type': 'string'}
})

sports_layout = deepcopy(extended_layout)
sports_layout['type']['allowed'] = ['sportsPin']
sports_layout['title']['required'] = True
sports_layout['tinyIcon']['required'] = True
sports_layout['largeIcon']['required'] = True
sports_layout.update({
    'rankAway': {'type': 'string'},  # TODO: Constraints
    'rankHome': {'type': 'string'},  # TODO: Constraints
    'nameAway': {'type': 'string'},  # TODO: Constraints
    'nameHome': {'type': 'string'},  # TODO: Constraints
    'recordAway': {'type': 'string'},  # TODO: Constraints
    'recordHome': {'type': 'string'},  # TODO: Constraints
    'scoreAway': {'type': 'string'},  # TODO: Constraints
    'scoreHome': {'type': 'string'},  # TODO: Constraints
    'sportsGameState': {'type': 'string', 'allowed': ('in-game', 'pre-game')},
})

weather_layout = deepcopy(extended_layout)
weather_layout['type']['allowed'] = ['weatherPin']
weather_layout['title']['required'] = True
weather_layout['tinyIcon']['required'] = True
weather_layout['largeIcon']['required'] = True
weather_layout.update({
    'locationName': {'type': 'string'}
})

generic_reminder_layout = deepcopy(basic_layout)
generic_reminder_layout['type']['allowed'] = ['genericReminder']
generic_reminder_layout['title']['required'] = True
generic_reminder_layout['tinyIcon']['required'] = True
generic_reminder_layout.update({
    'locationName': {'type': 'string'}
})

generic_notification_layout = deepcopy(basic_layout)
generic_notification_layout['type']['allowed'] = ['genericNotification']
generic_notification_layout['title']['required'] = True
generic_notification_layout['tinyIcon']['required'] = True

layouts = [generic_notification_layout, generic_layout, calendar_layout, sports_layout,
           weather_layout, generic_reminder_layout]

notification = {
    'layout': {'required': True, 'type': 'dict', 'oneof': [{'schema': l} for l in layouts]
    },
    'time': {'type': 'string'}
}

create_notification = notification

update_notification = deepcopy(notification)
update_notification['time']['required'] = True

reminder = {
    'time': {'type': 'string'},  # TODO: datetime
    'layout': {'type': 'dict', 'oneof': [{'schema': l} for l in layouts]},
}


action = {
    'title': {'type': 'string'},
    'type': {'type': 'string', 'allowed': ['openWatchApp']},
    'launchCode': {'type': 'integer'}
}

pin = {
    'id': {'type': 'string'},
    'time': {'required': True, 'type': 'string'},
    'duration': {'type': 'integer'},
    'createNotification': {'type': 'dict', 'schema': create_notification},
    'updateNotification': {'type': 'dict', 'schema': update_notification},
    'layout': {'type': 'dict',
        'oneof': [{'schema': l} for l in layouts],
        # 'oneof_schema': [generic_layout, calendar_layout,
        #                  sports_layout, weather_layout],
    },
    'reminders': {'type': 'list',
        'schema': {'type': 'dict', 'schema': reminder}
    },
    'actions': {'type': 'list', 'maxlength': 3,
        'schema': {'type': 'dict', 'schema': action}
    },

}
