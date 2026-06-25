# Lesson 4 — Sorting + Inspection Line (Setup Guide)

A 3-arm, 2-conveyor sorting line. **Three separate Arduino Mega boards**, each with
its own Dobot Magician, all in this one PlatformIO project.

```
[Start] --Arm1--> [Conveyor 1] --Arm2--> RED area
                               \--else--> [Conveyor 2] --Arm3--> GREEN area
                                                              \--blue/yellow--> Inspection + checker
```

| Board | Firmware (env) | What it does |
|-------|----------------|--------------|
| **Arm 1 – Feeder**  | `arm1_feeder`  | Picks each cube from the start point, drops it on Conveyor 1, runs the belt to Arm 2. |
| **Arm 2 – Sorter**  | `arm2_sorter`  | Red → red area; anything else → Conveyor 2 (runs the belt to Arm 3). Has a camera. |
| **Arm 3 – Inspect** | `arm3_inspect` | Green → green area; blue/yellow → inspection area. Runs the Mission 4 checker. Has a camera. |

---

## 1. Build & upload (do this per board)

Plug in **one** board, flash **its** firmware, repeat:

```powershell
pio run -e arm1_feeder  -t upload   # to the Feeder board
pio run -e arm2_sorter  -t upload   # to the Sorter board
pio run -e arm3_inspect -t upload   # to the Inspect board
```

`pio run` (no `-e`) just compiles all three without uploading.
> ⚠️ Flash the **right env to the right board** — they are not interchangeable.

---

## 2. Wiring / pin connections

**Every board** uses these:

| Signal | Pin | Notes |
|--------|-----|-------|
| Dobot Magician | **Serial1**: TX1=**18**, RX1=**19** | Mega 18→Dobot RX, 19→Dobot TX, + shared GND. 115200 baud. |
| Suction pump relay | **D7** | Pump on external **3.5 V** supply. |
| USB (debug) | Serial / USB | 115200 baud, for the Serial Monitor. |

**Arm 1 – Feeder** (also drives Conveyor 1 + start-point sensor):

| Signal | Pin |
|--------|-----|
| Conveyor 1 STEP | **D2** |
| Conveyor 1 DIR  | **D3** |
| Conveyor 1 EN   | **D4** (active LOW) |
| HC-SR04 TRIG | **D5** |
| HC-SR04 ECHO | **D6** (5 V board, connect directly) |

> The ultrasonic sensor detects when a cube is at the start point. Aim it at the
> fixture and make sure the arm's `HOME` pose does not sit in front of it.

**Arm 2 – Sorter** (also drives Conveyor 2 + camera):

| Signal | Pin |
|--------|-----|
| HuskyLens SDA | **20** (I2C) |
| HuskyLens SCL | **21** (I2C) |
| Conveyor 2 STEP | **D2** |
| Conveyor 2 DIR  | **D3** |
| Conveyor 2 EN   | **D4** (active LOW) |

**Arm 3 – Inspect** (camera + checking device, no belt):

| Signal | Pin |
|--------|-----|
| HuskyLens SDA | **20** (I2C) |
| HuskyLens SCL | **21** (I2C) |
| Green LED | **D8** |
| Red LED   | **D9** |
| Buzzer    | **D10** |
| Button    | **D11** — active **HIGH**, wire to 5 V with a **pull-down** resistor to GND |

> Conveyors use an **A4988 / DRV8825**-style step/dir driver. Different driver → edit `lib/Conveyor`.

---

## 3. Coordinates to calibrate  *(X, Y, Z, R in mm / degrees)*

These are **placeholder guesses** — replace them with the real positions.

**Easiest way — the built-in teach tool** (no DobotStudio needed):

```powershell
pio run -e getpos_arm2 -t upload     # flash the matching board
```

Then open the Serial Monitor (115200), **hold the unlock key on the arm**, move it
by hand to each spot, and read the printed `X= Y= Z= R=`. Paste those into the
`const` block at the top of that arm's sketch, then re-flash the real firmware
(`arm2_sorter`). Tools: `getpos_arm1`, `getpos_arm2`, `getpos_arm3`.

**Arm 1 – Feeder** (`src/arm1_feeder.cpp`)

| Name | X | Y | Z | R | Meaning |
|------|---|---|---|---|---------|
| START | 200 | 0 | -40 | 0 | pick-up point |
| BELT1 | 100 | -150 | -40 | 0 | drop onto Conveyor 1 |
| HOME  | 200 | 0 | 50 | 0 | safe ready pose |

**Arm 2 – Sorter** (`src/arm2_sorter.cpp`)

| Name | X | Y | Z | R | Meaning |
|------|---|---|---|---|---------|
| PICK  | 200 | 0 | -40 | 0 | end of Conveyor 1 |
| RED   | 150 | 150 | -40 | 0 | red area |
| BELT2 | 100 | -150 | -40 | 0 | load point of Conveyor 2 |
| HOME  | 200 | 0 | 50 | 0 | ready pose |

**Arm 3 – Inspect** (`src/arm3_inspect.cpp`)

| Name | X | Y | Z | R | Meaning |
|------|---|---|---|---|---------|
| PICK    | 200 | 0 | -40 | 0 | end of Conveyor 2 |
| GREEN   | 150 | 150 | -40 | 0 | green area |
| INSPECT | 250 | -120 | -40 | 0 | inspection area |
| HOME    | 200 | 0 | 50 | 0 | ready pose |

Shared per sketch: `LIFT_HEIGHT = 60` mm (lift after grabbing).

---

## 4. Conveyor travel calibration

Each belt runs a **fixed number of steps** (open-loop) so a cube travels from the
load point to the next arm's detection point, then stops.

- `BELT1_TRAVEL_STEPS` in `src/arm1_feeder.cpp` (default **4000**)
- `BELT2_TRAVEL_STEPS` in `src/arm2_sorter.cpp` (default **4000**)

**How to calibrate:** put a cube at the load end, run the belt, count/adjust steps
until the cube stops right under the next arm's camera. Tune the constant, re-flash.

---

## 5. HuskyLens training (Arm 2 and Arm 3)

Set the HuskyLens **Protocol = I2C** (General Settings) first. Then:

**Colour Recognition mode** — learn the cubes in this exact ID order:

| ID | Colour |
|----|--------|
| 1 | red |
| 2 | green |
| 3 | blue |
| 4 | yellow |

**Tag Recognition mode** (Arm 3 only) — learn April-tag **TAG36H11-0** as **ID 1**
(this is the buzzer trigger).

---

## 6. Mission 4 — checking device rules (Arm 3)

Implemented in `checkInspectionArea()`:

1. **≥1 blue AND ≥1 yellow** in the area → **green LED on**, else **red LED on**.
2. **any red block OR the April tag** present → **buzzer on** (`tone` 500 Hz).
3. **press the button** → buzzer off (re-arms on the next check if the cause remains).

> ⚠️ One HuskyLens can only look at one spot. Arm 3's camera is shared between the
> conveyor pickup and the inspection area, so the checker runs **once per cube**, not
> continuously. For continuous monitoring, give the checker its **own** HuskyLens.

---

## 7. Pre-run checklist (consider before powering the line)

- [ ] **Home each Dobot once in DobotStudio** — this SDK has no auto-home, so the
      coordinates are only correct after a manual home.
- [ ] Calibrated all coordinates (Section 3) for each arm.
- [ ] Calibrated `BELT*_TRAVEL_STEPS` (Section 4).
- [ ] Arm 1 ultrasonic wired (TRIG **D5** / ECHO **D6**) and `BLOCK_DISTANCE_CM` calibrated.
- [ ] HuskyLens trained + Protocol = I2C (Section 5).
- [ ] Button wired **active-HIGH** with a pull-down resistor.
- [ ] Pump relay on **D7**, pump on its **3.5 V** supply.
- [ ] Each arm's `HOME` pose is clear of the belts and other arms (no collisions).
- [ ] Correct firmware on each board (`arm1`/`arm2`/`arm3`).
- [ ] Open the Serial Monitor (115200) to watch the `colour ID = …` debug prints.

**Timing reminders:** Arm 1 waits for the **ultrasonic sensor** to see a cube at
the start point before feeding (calibrate `BLOCK_DISTANCE_CM`); there are ~3 s
settle delays between every arm move; suction is held 2 s before lifting. Don't
expect instant motion.

---

## 8. (Optional) board-to-board communication — not wired yet

If you later want the boards to talk (e.g. "cube arrived, stop the belt"):

- Use **Serial2** (TX2=**16**, RX2=**17**) or **Serial3** (TX3=**14**, RX3=**15**).
  **Do NOT use Serial1 (18/19) — that's the Dobot.**
- Cross-wire TX→RX, RX→TX, and **share GND** between the boards.
- 5 V logic both sides, so no level shifter needed.

Ask and I'll add the `Serial2`/`Serial3` send/receive code once you decide the messages.

---

## Project layout

```
src/   arm1_feeder.cpp   arm2_sorter.cpp   arm3_inspect.cpp
lib/   DobotArm/    (wraps the SDK: dobotInit / goTo / dobotGetPose)
       DobotSDK/    (official Dobot Magician protocol, Serial1)
       Conveyor/    (A4988 step/dir belt driver)
       Ultrasonic/  (HC-SR04 distance sensor, used by Arm 1)
reference/  Lesson2_template/  Lesson3_template/   (original class files, not built)
platformio.ini   (3 build environments)
```
