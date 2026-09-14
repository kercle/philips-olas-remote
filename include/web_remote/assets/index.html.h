#pragma once

#include <Arduino.h>

namespace assets {

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">

    <meta name="theme-color" content="#151515">

    <title>Ceiling Fan</title>

    <style>
        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            min-height: 100vh;
            background: #151515;
            color: #fff;
            font-family: -apple-system, BlinkMacSystemFont,
                "Segoe UI", sans-serif;

            display: flex;
            align-items: center;
            justify-content: center;
            padding: 24px;
        }

        .panel {
            width: 100%;
            max-width: 430px;
        }

        h1 {
            font-size: 30px;
            margin: 0 0 30px;
            text-align: center;
            font-weight: 600;
        }

        button {
            display: block;
            width: 100%;
            height: 76px;

            margin: 16px 0;

            border: none;
            border-radius: 18px;

            font-size: 21px;
            font-weight: 600;

            background: #303030;
            color: white;

            -webkit-tap-highlight-color: transparent;
        }

        button:active {
            transform: scale(0.98);
            background: #444;
        }

        .light-on {
            background: #e4c45c;
            color: #111;
        }

        .off {
            background: #303030;
        }

        #status {
            margin-top: 25px;
            height: 24px;
            text-align: center;
            color: #999;
            font-size: 14px;
        }
    </style>
</head>

<body>
    <div class="panel">
        <h1>Ceiling Fan</h1>
        <button class="light-on" onclick="sendCommand('/api/light/on')">
            Light On
        </button>
        <button class="off" onclick="sendCommand('/api/light/off')">
            Light Off
        </button>
        <button class="off" onclick="sendCommand('/api/fan/off')">
            Fan Off
        </button>
        <button class="off" onclick="sendCommand('/api/fan/1')">
            Fan 1
        </button>
        <button class="off" onclick="sendCommand('/api/fan/3')">
            Fan 3
        </button>
        <button class="off" onclick="sendCommand('/api/fan/6')">
            Fan 6
        </button>
        <div id="status"></div>
    </div>
    <script>
        async function sendCommand(url) {
            const status =
                document.getElementById('status');
            status.textContent = 'Sending…';

            try {
                const response = await fetch(url, {
                    method: 'POST',
                    cache: 'no-store'
                });
                const text = await response.text();

                if (response.ok) {
                    status.textContent = text;
                } else {
                    status.textContent = 'Error: ' + text;
                }
            } catch (e) {
                status.textContent =
                    'Connection failed';
            }
        }
    </script>
</body>

</html>
)rawliteral";

}
