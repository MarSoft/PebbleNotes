pypebbleapi
============
[![Build Status](https://travis-ci.org/youtux/pypebbleapi.svg?branch=master)](https://travis-ci.org/youtux/pypebbleapi)
[![Documentation Status](https://readthedocs.org/projects/pypebbleapi/badge/?version=latest)](http://pypebbleapi.readthedocs.org/en/latest)

[Pebble Timeline](http://developer.getpebble.com/guides/timeline/) APIs for python.

This is a library to ease the access to the Pebble Timeline and validate pins.
It supports Python 2.7, 3.3 and 3.4.

Update
-----
Starting from version *1.0.0*, the API has changed. The `Pin` class has
been removed. You should now supply a `dict`, which will be validated before sending.

Install
-------

Just like you install any package:

    $ pip install pypebbleapi

Usage
-----

Here's an example:
```python
from pypebbleapi import Timeline
import datetime

timeline = Timeline(
    api_key=my_api_key,  # Needed only if you are going to use shared pins
)

my_pin = dict(
    id='123',
    time=datetime.date.today().isoformat(),
    layout=dict(
        type="genericPin",
        title="This is a genericPin!",
        tinyIcon="system://images/NOTIFICATION_FLAG",
    )
)

# Send a shared pin
timeline.send_shared_pin(
    topics=['a_topic', 'another_topic'],  # List of the topics
    pin=my_pin,
)

# Send a user pin
timeline.send_user_pin(
    user_token='test-user-token',
    pin=my_pin,
)
```
It is possible that **validation fails** even if the pin is correct (it could happen if Pebble updates the pin specification).
In this case you may want to skip the validation:
```python
timeline.send_user_pin(
    user_token='test-user-token',
    pin=my_pin,
    skip_validation=True,
)
```

Error handling
-----
The API raises errors in case the server is not available or if it returns error codes. You should always enclose calls in `try/except`:
```python
try:
    timeline.send_shared_pin(...)
except Exception as e:
	print(e)
```

If the pin you provided is not valid, a `DocumentError` will be raised:
```python
from pypebbleapi import DocumentError

bad_pin = {}  # Empty pin is not valid
try:
    timeline.send_shared_pin(['a_topic'], bad_pin)
except DocumentError as e:
    print(e)
    print(e.errors)  # e.errors contain a dictionary of the fields that failed the validation
```

