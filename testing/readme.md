## Test enviroment

For simplicity, the project has been mounted on a protoboard and Arduino has been connected to a computer to receive power from the USB port, as follows.

![Project](https://raw.githubusercontent.com/dnatividade/Arduino_mousejack/master/testing/project.jpg)


The victim was a Windows 10 64-bit laptop and a Logitech xxx mouse, as shown in the image below.

![Mouse](https://raw.githubusercontent.com/dnatividade/Arduino_mousejack/master/testing/mouse.jpg)


After some mouse interactions, Arduno is able to access the victim's mouse dongle and send commands like keyboard keys, as shown in the video below.

![Video](./working.webm)

---

## Dificuties found

 The communication was very unstable because sometimes the command could be sent, sometimes not.

 The test was done in a location away from other radio connections (2.4 GHz) to eliminate interference problems. Even so, the result was the same.

 In most tests, only part of the command can reach the attacked computer, for example:

 In testing, the following keystrokes were sent:
```
WIN + r (to open "run")
cmd [ENTER]
shutdown -f -s -t 0 [ENTER]
```

But at the destination came only "shutdo" (from the "shutdown" command).
Other times, not even the "shutdown" command reached the attacked computer.

For this reason, the project was codenamed "ShutDô".

---

The next test will be performed with RF-Nano (an arduino + Nordic rf24 built-in device) to eliminate interference issues and poor connection contact.
![RF-Nano](https://raw.githubusercontent.com/dnatividade/Arduino_mousejack/master/img/rf-nano.jpeg)

**Soon we will have more results ...**
