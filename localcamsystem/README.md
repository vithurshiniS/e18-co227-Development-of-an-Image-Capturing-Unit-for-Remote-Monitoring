
# Monitor cams using Local Server

esp32 cam specification is not enough to serve multiple clients at same time. There fore we used Node.js to build the
server which can be used as to recieve the frames from esp32cam and send them to multiple clients.

<p>&nbsp;</p>

## Run Locally
<p>&nbsp;</p>

Clone the project

```bash
  git clone https://github.com/cepdnaclk/e18-co227-Development-of-an-Image-Capturing-Unit-for-Remote-Monitoring.git
```

Go to the project directory

```bash
  cd e18-co227-Development-of-an-Image-Capturing-Unit-for-Remote-Monitoring.git/localcamsystem/server
```
Init Node project
```bash
  npm init
```

Install dependencies

```bash
  npm i
```

Start the server

```bash
  npm run start
```

