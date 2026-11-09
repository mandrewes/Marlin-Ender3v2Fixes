import schemdraw, schemdraw.elements as e
schemdraw.config(fontsize=12)
with schemdraw.Drawing(file='adapter_circuit.png', dpi=160, show=False) as d:
    d += e.Vdd().label('FAN+  (+24 V, board fan socket)')
    d += e.Resistor().down().label('Rload\n2.2 kΩ 1 W', loc='bottom')
    d += e.Dot()
    fan = d.here
    d += e.Line().left(1.5)
    d += e.Dot(open=True).label('FAN−  (switched by\nboard MOSFET, PA0)', loc='left')
    d.here = fan
    d += e.Line().down(0.5)
    d += e.Resistor().down().label('R1 47 kΩ', loc='bottom')
    d += e.Dot()
    a = d.here
    d += e.Resistor().down().label('R2 10 kΩ', loc='bottom')
    d += e.Ground()
    d.here = a
    d += e.Resistor().right().label('Rb 4.7 kΩ')
    q = e.BjtNpn(circle=True).anchor('base').label('Q1\n2N3904', loc='left', ofst=(-0.1,-0.9))
    d += q
    d += e.Line().at(q.emitter).down(0.5)
    d += e.Ground()
    d += e.Line().at(q.collector).up(1.5)
    d += e.Dot()
    out = d.here
    d += e.Resistor().up().label('Rc 1 kΩ', loc='bottom')
    d += e.Vdd().label('+5 V (from 24→5 V buck)')
    d.here = out
    d += e.Line().right(2.5)
    d += e.Dot()
    o2 = d.here
    d += e.Resistor().down().label('Rpd\n10 kΩ', loc='bottom')
    d += e.Ground()
    d.here = o2
    d += e.Resistor().right().label('Rs 220 Ω')
    d += e.Dot(open=True).label('Laser PWM / TTL in', loc='right')
    d += e.Label().at((-3, -12.5)).label(
        'Fan output ON  → FAN− ≈ 0 V → Q1 off → OUT ≈ 4.5 V (laser ON)\n'
        'Fan output OFF → FAN− ≈ 24 V → Q1 on → OUT ≈ 0 V (laser OFF)\n'
        'No 5 V / board off → Rpd holds OUT low\n'
        'All grounds common: PSU DC−, buck GND, laser GND.  Optional 100 nF across +5 V–GND.', fontsize=12, halign='left')
