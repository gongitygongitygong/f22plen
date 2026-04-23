# f22plen

yeah so this is a fake jet engine / EDF / turbine simulator thing running on an Arduino.

it does:
- fake engine spool physics
- fake ignition
- fake starter motor behaviour
- fake flameouts if you mess with fuel while engine on
- elevon control for some reason because i started this as an aircraft thing and now it’s this (was meant to fly at some pt btw)


---

## what this is NOT

- not a real engine controller
- not flight certified
- not safe for anything important
- not accurate aerospace simulation
- not something you should bet your life on
- if it blows up not my problem

---

## hardware pins (aka where things are plugged in)

- A0 → proportional roll input
- A1 → proportional pitch input
- A2 → proportional throttle input

- D4 → fuel pump switch (HIGH = off, LOW = on because pullup logic)
- D3 → starter button (hold to start spooling)
- D2 → ignition button (press to start engine)

- D11 → left elevon servo
- D10 → right elevon servo
- D6 → throttle output servo/esc or wtv u use for throttle

---

## how it works

### engine states

- OFF → nothing is happening, engine do a sleepy
- STARTING → starter spins engine up slowly
- RUNNING → engine is alive and you can use throttle
- SHUTDOWN → engine is dying

---

## main ideas in the code

### starterSpool
this is basically how fast the turbine is spinning before it starts

it builds up slowly and decays slowly because inertia is lowk a thing

---

### engineRPM
this is just a number that feels like engine power

it is NOT real RPM. pls no sue

---

### rpmTarget
this is what the engine wants to be at

engineRPM slowly follows this but not really accurately cos inertia and stuff

---

### ignition
press button → wait a bit → engine starts

there is a delay because real engines don’t instantly care about your inputs

---

### flameout
if you turn fuel off while running (ha ha funny prank)
- engine doesn’t instantly die
- it loses power
- then slowly gives up and shuts down
  
---

## important behaviour notes

- ignition only works when engine is already spooled up enough
- throttle only matters after engine is running (no effect if engine off)
- everything is delayed slightly because instant response feels fake

---

## tuning knobs if you wanna mess with it

in code you’ll find stuff like:

- `engineInertiaUp` → how lazy acceleration is
- `engineInertiaDown` → how slow it dies
- `starterAccel` → how weak your starter motor is
- `IGNITION_DELAY` → how long before it actually starts

---

## why this exists

my sch open house project but i put a lil too much effort so here we are

---

## possible future upgrades

- compressor stall if you throttle wrong
- startup failure chance
- EGT temperature simulation
- fake turbine sound PWM output
- auto ECU mode so humans are not needed anymore (not fun tho so prolly no)

---

## final note

if this catches fire it is not my fault. pls check wiring make sure correct.
