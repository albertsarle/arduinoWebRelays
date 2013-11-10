arduinoWebRelays
================

This is the last version of my arduino prototype to remotely switch on/ off the central heating and the hot water accumulator. The main purpose is to monitor inside & outside temperature, display the current status of the switches because someone else might have pushed the buttons, and allow to update the status.

The sainsmart relays will control two standard plugs where the heating and the hot water accumulator will be connected. The two outputs left are available to control something else, probably some outdoor lights. The two temperature sensors are the LM35 and a DHT11 which also has a humidity sensor and will be the on the outside.

The arduino program makes use of interruptions to process the switch events, and displays the current status in a basic html page which is served directly from the arduino. All the css styles and javascript events are loaded from external sources to minify the response handled by the arduino, so the binary skecth size is just 20790 bytes.

The events of the buttons in the webpage are handled by jquery ajax calls and sent back to arduino faking an API url which just updates the status and doesn't rewrite the entire page.

The next step is to wire everything inside a nice box, wire the external connections of the temperature sensors and connect the socket.
