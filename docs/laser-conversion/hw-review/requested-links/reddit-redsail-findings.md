# Requested Reddit and Redsail link review

Research only; no firmware, repository, SD, wiring or hardware changes.

## Access and source status
- Reddit direct page opened, but its reader extraction returned only the opening question, not comments. old.reddit public page timed out. Search indexing exposed specific original-thread comments; the complete comment tree was NOT obtained.
- Original thread: https://www.reddit.com/r/ender3v2/comments/imlsb8/someone_try_the_laser_engraver_kit_in_the_ender_3/
- Imgur album linked by a success report returned HTTP 429; images not verified: https://imgur.com/a/2oNuiWV
- Redsail article was read in full: https://www.redsaillaser.com/archives/5529.html
- Followed and read the actual adapter article linked by Reddit: https://blog.pasqualefiorillo.it/3d-printer-pwm-ttl-laser-adapter/
- Adapter schematic previously downloaded and visually examined: /workspace/laser-hw-review/conversions/published-opto-adapter.png

## Actual Reddit evidence
BaMbaRE says: “I'm in the process of setting up 1W (12V, PWM) laser on V2.” They report discovering 24 V fans, link Fiorillo's PWM/TTL adapter, say “I set R2 to 1kOm”, and state electrical wiring is done while they are still finding a mount. This is evidence for a small adapter circuit, not a calibrated waveform or completed engraving proof.
vinicaog separately says their module works with Ender 3 V2 and links the unavailable Imgur album. Other comments discuss mounts and working laser burning with positioning problems.
Exact PCB revision and precise laser model are not established in accessible comments. Do not promote these into a verified V4.2.2/Snapmaker-specific schematic.

## Actual linked circuit
Fiorillo explicitly requires separate laser power and PWM/TTL ports. The schematic shows a 4N35 optocoupler, R1=2.2 kohm for the 24 V fan input, R2=470 ohm on the optotransistor emitter/output, a +5 V collector supply from an L7805 regulator, diode and bypass capacitors, and a fan/laser selector. Blog text says 1.1 kohm at 12 V while schematic annotation says 1 kohm; only 24 V setting matters here. Reddit user's R2=1 kohm is their reported change, NOT a proposed change to R1.
Input LED circuit is connected ACROSS fan + and switched fan negative. With low-side switch ON, it lights; optotransistor produces active-high approximately 5 V-derived logic across output resistor. This is an interface conversion, not direct 24 V-to-MCU wiring. Laser power is separate. The pictured connector polarity is explicitly for Ender 3 PRO; not a verified V4.2.2 physical contact orientation.
A previous independent forum review identified a selector/jumper concern in an earlier version; do not present downloaded artwork as automatically approved construction files. 4N35 switching performance and output polarity require verification at the intended frequency/load.

## Redsail claims and deficiencies
Article says connect power/ground to 24 V fan or hotend terminals and PWM wire to “fan header (D9)”; firmware section says LASER_PWM_PIN is “usually D9”. It names generic 5.5 W / 10 W modules but supplies no exact module, PCB revision, circuit schematic, component values, measured waveform or verified V4.2.2 contact map. A V4.2.2 fan header is switched 24 V power, not bare logic; its published PA0 control does not validate this unqualified D9 instruction. Thus this source supports the general possibility of conversions, not direct TTL wiring or a resistor-only fix.
No pull-down, pull-up or divider circuit is actually specified in that article.

## Engineering interpretation (not source quotations)
A two-wire brushless fan is an electronic load with possible input capacitance. Across a PWM supply, it can hold its own supply between pulses and distort the low-side drain OFF rise; it is not a clean resistive scope-test load. If its voltage stays near 24 V across the fan, the switched drain relative to board ground should instead be near 0 V during held-up intervals. A trace sitting near 24 V at a correctly referenced drain while that fan continuously has near 24 V across it is not consistent with the simple intended topology; first establish which two nodes were measured. This does not establish the root cause.
A defined load resistor from output +24 V to switched output negative is a pull-UP/load for that drain, not a pull-down to ground. It can make the OFF rise well defined, but it cannot repair a MOSFET that never switches or wrong firmware routing. A pull-down from drain to board ground biases the drain in the wrong direction for this diagnostic and creates an alternate return.
An engineered diagnostic example, not from either linked source: 2.2 kohm across output +/minus draws 24/2200=10.9 mA and dissipates 24^2/2200=0.262 W at continuous ON. A suitably mounted 1 W resistor offers thermal margin but can still warm. This is only a small diagnostic load, not a test of full fan-current capacity, and not a laser-interface approval.
Use one output, no fan/heater/laser in parallel, power off for connection changes, and scope ground ONLY on confirmed DC negative. First distinguish static commanded endpoints: across load ideally OFF~0 V / ON~24 V; switched drain to ground ideally OFF~24 V / ON~0 V. Only then interpret 1 kHz waveform. Keep SCOPE9 software audit independent.

## What changes / does not change
The Reddit evidence validates Michael's suggestion that a few interface components can indeed be sufficient for a compatible laser driver: an optocoupler, resistors and a logic supply are explicitly linked. It does not show that a lone pull-down fixes the current 50 mV waveform. Neither page validates direct use of the PA7 endstop socket, whose 10 kohm series/pull-up network already explains elevated LOW, nor direct compatibility with the parked active-low Snapmaker interface.
