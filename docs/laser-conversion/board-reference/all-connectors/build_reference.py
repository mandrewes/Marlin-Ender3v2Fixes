from pathlib import Path
import json,csv,base64,html,textwrap
from PIL import Image
import cairosvg
from reportlab.pdfgen import canvas
from reportlab.lib.pagesizes import A4,landscape
from reportlab.lib import colors
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle
P=Path('/workspace/laser-hw-review/all-connectors')
# Functional nets, NOT a physical contact numbering scheme.
R=[]
def add(n,name,count,function,electrical,mcu,pwm,notes,source):
 R.append(dict(id=f'{n:02}',identifier=name,contacts=count,function=function,electrical=electrical,mcu=mcu,pwm=pwm,notes=notes,sources=source))
add(1,'24 V IN','2 screw terminals','Main DC supply input','24 V DC, positive and board ground','None','NO: supply input','Not mains. Physical screw polarity must follow the actual board markings.','S1, S2')
add(2,'FAN + / - (fixed-power pair)','2 screw terminals','Always-powered hotend/heatsink fan supply','24 V DC; permanent ground return','None','NO: fixed supply','Not K-FAN1/2. Fan fuse/rail routing differs between published drawings and manufacturing reports; do not assume which fuse protects it.','S1, S2')
add(3,'BED + / -','2 screw terminals','Heated-bed power output','24 V positive; MOSFET-switched negative','PA2 -> buffer -> bed MOSFET','POWER PWM / heater switching; not TTL','PA2 supports native timer channels at the MCU, but those are not exposed here. Negative terminal is not permanent ground.','S1, S3, S4')
add(4,'NOZ + / - (hotend heater)','2 screw terminals','Original nozzle-heater output; current test laser-enable output','24 V positive; MOSFET-switched negative','PA1 -> buffer -> nozzle MOSFET','POWER PWM capable; current laser test uses ON/OFF enable','PA1 is not raw logic at the screws. Common ground tied to its switched negative can bypass the power switch. No heater or laser connection recommended.','S1, S3, S4; current test context')
add(5,'K-FAN1','2-pin socket','Controlled fan output','24 V positive; shared low-side MOSFET return','PA0 -> buffer -> fan MOSFET','POWER PWM, not TTL','Shares one control/net with K-FAN2: not independent channels. Marlin declares FAN_SOFT_PWM_REQUIRED. Current SCOPE6 context reports about 7.8 Hz software PWM, not laser-frequency PWM.','S1, S3; current test context')
add(6,'K-FAN2','2-pin socket','Second controlled fan output (often board-cooling fan)','24 V positive; same switched return as K-FAN1','PA0, shared with K-FAN1','POWER PWM, not TTL','Same command/duty as K-FAN1; not a separate spare output. Do not confuse it with the fixed FAN screw pair.','S1, S3')
for n,axis,step,direction in [(7,'X','PC2','PB9'),(8,'Y','PB8','PB7'),(9,'Z','PB6','PB5'),(10,'E','PB4','PB3')]:
 add(n,axis+' MOTOR','4-pin socket',axis+' stepper-motor coil connections','Two current-regulated H-bridge coil pairs; approximately 24 V switching, not a DC supply',f'{step} STEP, {direction} DIR; PC3 shared ENABLE (upstream of driver)','NO usable logic PWM at socket; internal driver chopper switching is NOT a laser signal','MCU STEP/DIR pins are not the motor socket contacts. Driver type varies by V4.2.2 manufacturing run. All four enables share PC3.','S1, S3')
for n,axis,pin,series,pull,cap,af in [(11,'X','PA5','R11','R4','C11','No native timer PWM on PA5'),(12,'Y','PA6','R12','R5','C12','PA6 can carry TIM3_CH1 at the MCU'),(13,'Z','PA7','R13','R6','C13','PA7 can carry TIM3_CH2 / TIM8_CH1N at the MCU')]:
 add(n,axis+'- ENDSTOP','2-pin socket',axis+' minimum limit-switch input','Signal plus ground; 3.3 V input network, NO supply contact',pin,'INPUT as wired; not a clean logic-PWM output',f'{series}=10 kohm series, {pull}=10 kohm pull-up to 3.3 V, {cap} to ground (100 nF ASSUMED by schematic author). {af}. Divider can hold a driven-low socket near 1.65 V. '+('User measured about 3.3 V HIGH / 1.6 V LOW and rounded ~844 mVpp at ~1 kHz; this is user-reported, not a new measurement.' if axis=='Z' else ''),('S1, S3, S4; Z measurements supplied by user' if axis=='Z' else 'S1, S3, S4'))
add(14,'J1 / filament runout','3-pin socket','Optional filament detector','PA4 signal, ground, sensor-supply contact','PA4','INPUT as wired; PA4 has NO native timer-PWM alternate function','R10=10 kohm series; R3=10 kohm pull-up; no signal capacitor shown. V4.2.2 schematic/wiki specify 3.3 V supply, but a combined TH3D page illustrating V4.2.7 says 5 V. Actual rail unmeasured. No outer-contact physical order asserted.','S1, S3, S4, S5, S6')
add(15,'BL_T / probe (G V IN G OUT)','5-pin socket','3D Touch / BLTouch-compatible probe power, deploy and trigger','Two grounds, 5 V supply, 3.3 V MCU control/trigger nets','PB0 deploy/control; PB1 trigger','SERVO control pulses on PB0; PB1 is trigger INPUT','Occupied by working 3D Touch: KEEP. PB0/PB1 have timer alternate functions, but are not spare laser pins. No series/endstop RC network shown in these probe signal paths; not permission to attach a laser.','S1, S3, S4, S5; current wiring context')
add(16,'TB / bed thermistor','2-pin socket','Bed-temperature sensor input','Analog input plus ground, 3.3 V pull-up/filter network','PC4','NO native timer PWM; analog sensing input','Reference: R58=4.7 kohm pull-up, C41 labelled 10 uF. Not a power output. Sensor connector positions can be mislabelled in marketing images; PCB silk and trace take precedence.','S1, S3, S4')
add(17,'TH / hotend thermistor','2-pin socket','Nozzle-temperature sensor input','Analog input plus ground, 3.3 V pull-up/filter network','PC5','NO native timer PWM; analog sensing input','Reference: R57=4.7 kohm pull-up, C40 labelled 10 uF. Not a power output. Capacitance values are reference-drawing values, not measurements of this board.','S1, S3, S4')
add(18,'Display / EXP (IDC ribbon)','10-pin shrouded socket','Ender-3 V2 display serial link, encoder/button/beeper signals','3.3 V MCU signals plus 5 V display supply and ground','RET6 map: PC6, PB2, PB10, PB11, PB14, PB13, PB12, PB15','NOT a stock spare PWM connector; some nets have timer alternate functions','PB10/PB11 are USART3; display uses other nets for controls/beeper. Schematic includes 47-ohm series networks. Occupied display signals cannot simply be reassigned. No numbered ribbon-pin orientation given here.','S1, S3, S5')
add(19,'USB1 / micro-USB','5 USB contacts plus shell','Computer serial connection through CH340G bridge','USB differential D+/D-, VBUS, ground; not a raw MCU USB/PWM header','CH340G -> MCU PA9 TX / PA10 RX (USART1)','NO: USB/serial interface','USB can partially power logic; do not assume board is dead merely because 24 V is off. USB D+/D- do not expose PA11/PA12; those MCU pins serve the onboard EEPROM here.','S1, S3')
add(20,'microSD socket','8 card contacts plus card-detect switch','Firmware/print-file storage','3.3 V SDIO bus','PC8..PC11 data; PC12 clock; PD2 command; PC7 detect','NO usable stock PWM interface','Storage bus contacts are not spare PWM outputs. SDIO uses these nets; PA4 is runout, not this socket chip-select.','S1, S3')
add(21,'P2 / SWD programming pads','4 pads; header not fitted in reference photo','Debug/programming interface','3.3 V reference, SWDIO, SWCLK, ground','PA13 SWDIO; PA14 SWCLK','NO native timer PWM on PA13/PA14','Schematic calls this ICSP; on STM32 it is SWD, not AVR ICSP. Pads, not a fitted four-pin socket. Do not assume the voltage pad is 5 V.','S1, S3, S4; photo P2 silk')
add(22,'P3 auxiliary pads ("Reset" in drawing)','2 pads; header not fitted','Auxiliary pulled-up input/test-pad pair','PC0 signal plus ground; 3.3 V pull-up/filter','PC0 (MCU pin 8) in traced circuit, NOT NRST pin 7','NO native timer PWM on PC0','Drawing labels it P3 Reset, but its traced net and author correction identify PC0. R60=10 kohm pull-up, R61=0 ohm link, C39 to ground (value assumed). Treat reset naming cautiously; no shorting instructions.','S1, S2, S4; photo P3 silk')
(P/'inventory.json').write_text(json.dumps(R,indent=2))
with (P/'inventory.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=R[0].keys());w.writeheader();w.writerows(R)
S=[('S1','Community traced V4.2.2 schematic, 28-5-22; NOT an official Creality schematic','https://github.com/Jyers/Marlin/files/8791332/Creality.4.2.2.-.Schematic.28-5-22.pdf'),('S2','Schematic author discussion/corrections, including PC0 at P3 and grounded MOSFET sources','https://github.com/Jyers/Marlin/discussions/814'),('S3','Marlin upstream V4 pin map; RET6 configuration, not V4.2.7 overrides','https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/Marlin/src/pins/stm32f1/pins_CREALITY_V4.h'),('S4','ST STM32F103xC/D/E manufacturer datasheet, alternate-function Table 5','https://www.st.com/resource/en/datasheet/stm32f103re.pdf'),('S5','Independent V4.2.2 community photo and J1/probe/display documentation','https://ender3.club/Creality_4.2.2'),('S6','Combined TH3D 4.2.2/4.2.7 article: illustrates 4.2.7; conflicting J1 supply claim','https://tickets.th3dstudio.com/products/article/creality-v4-2-7-v4-2-2-wiring-and-pinout-diagram'),('Photo','Original photograph visibly labelled V4.2.2; reference board, not a newly captured photo of Michael\'s unit','https://ender3.club/images/3/34/Pinount_creality_4.2.2.png')]
md=['# Creality V4.2.2 — complete connector inventory','\nReference: STM32F103RET6 Ender-3 V2. Callouts identify connectors, NOT physical pin numbers. Research-only; no changes made.','\nHardware timer capability, stock socket function and current test firmware are different things. Inputs are not automatically outputs. All capacitor values and manufacturing variants require appropriate caution.']
for r in R:
 md += [f"\n## {r['id']} — {r['identifier']}"]+[f"- **{k.replace('_',' ').title()}:** {v}" for k,v in r.items() if k not in ['id','identifier']]
md+=['\n## Evidence / limitations','High confidence: connector identity/counts, common GPIO assignments and power-versus-logic distinction. Medium/reference-level confidence: component values and precise rails on an individual manufacturing variant. J1 rail and physical end-contact orientation remain unresolved. P3 Reset is a misleading drawing label; not verified as an NRST connection.','Fuses F1/F2/F3, LEDs, ICs, crystals, resistors and onboard EEPROM are not external connectors. No additional three-pin endstop sockets or undocumented spare power headers were invented.','The reference photo exposes 20 connector/terminal interfaces plus 2 unpopulated pad groups = 22 callouts. No separate fitted reset switch or external I2C socket was identified.','\n## Sources']
md += [f'- {a}: {b}. {u}' for a,b,u in S]
(P/'inventory.md').write_text('\n'.join(md))
# Editable SVG: embedded unaltered reference photo plus vector markers and external keyed labels.
photo=Path('/workspace/laser-hw-review/three-pin-identification/community-422-labelled.png')
data=base64.b64encode(photo.read_bytes()).decode();svg=[]
def e(x):return html.escape(str(x))
def tx(x,y,t,size=27,color='#143249',bold=False):
 svg.append(f'<text x="{x}" y="{y}" font-size="{size}" fill="{color}" font-weight="{"bold" if bold else "normal"}">{e(t)}</text>')
svg.append('<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="2600" height="1740" viewBox="0 0 2600 1740"><rect width="2600" height="1740" fill="white"/><g font-family="DejaVu Sans, sans-serif">')
tx(50,65,'CREALITY V4.2.2 / ENDER-3 V2 — ALL CONNECTORS',43,bold=True)
tx(50,112,'22 keyed callouts: 20 connector/terminal interfaces + 2 unpopulated pad groups',29)
tx(50,155,'RESEARCH REFERENCE • callout numbers are NOT contact numbers • no wiring or firmware changes',25,'#a22626')
svg.append(f'<image x="610" y="300" width="1382" height="998" xlink:href="data:image/png;base64,{data}"/>')
coords={1:(42,411),2:(130,439),3:(204,439),4:(278,439),5:(354,351),6:(353,392),7:(101,36),8:(232,36),9:(366,36),10:(492,36),11:(410,450),12:(465,450),13:(515,450),14:(350,450),15:(550,372),16:(566,450),17:(615,450),18:(635,327),19:(640,178),20:(610,86),21:(585,246),22:(423,378)}
for n,(x,y) in coords.items():
 x=610+2*x;y=300+2*y
 svg.append(f'<circle cx="{x}" cy="{y}" r="26" fill="#124f77" stroke="white" stroke-width="4"/>')
 svg.append(f'<text x="{x}" y="{y+9}" text-anchor="middle" fill="white" font-size="27" font-weight="bold">{n:02}</text>')
left=[('01  24 V DC INPUT','2 screws • fixed DC supply'),('02  FIXED FAN + / -','2 screws • always-on 24 V'),('03  BED HEATER','2 screws • switched-power output'),('04  NOZZLE HEATER','2 screws • switched-power output'),('05  K-FAN1','2-pin • controlled 24 V power'),('06  K-FAN2','2-pin • shares K-FAN1 control')]
for i,(a,b) in enumerate(left):
 y=355+i*140;tx(35,y,a,29,bold=True);tx(35,y+42,b,24)
for i,(a,b,c) in enumerate([('18  DISPLAY / EXP','10-pin IDC • UART + controls','Occupied; not spare PWM'),('19  MICRO-USB','USB-to-serial bridge','Not raw MCU PWM'),('20  MICROSD','SDIO storage interface','Not spare PWM'),('21  P2 / SWD PADS','4 unpopulated debug pads','3.3 V / PA13 / PA14 / GND'),('22  P3 AUX PADS','2 pads • PC0 + GND','"Reset" label is misleading')]):
 y=355+i*170;tx(2030,y,a,28,bold=True);tx(2030,y+42,b,24);tx(2030,y+77,c,23)
for i,(a,b) in enumerate([('07  X MOTOR','4-pin coil output'),('08  Y MOTOR','4-pin coil output'),('09  Z MOTOR','4-pin coil output'),('10  E MOTOR','4-pin coil output')]):
 x=650+i*340;tx(x,230,a,28,bold=True);tx(x,270,b,23)
bot=[('11  X- ENDSTOP','2-pin input • PA5'),('12  Y- ENDSTOP','2-pin input • PA6'),('13  Z- ENDSTOP','2-pin input • PA7'),('14  J1 RUNOUT','3-pin input • PA4'),('15  BL_T PROBE','5-pin • PB0 / PB1 • occupied'),('16  TB THERMISTOR','2-pin analog input • PC4'),('17  TH THERMISTOR','2-pin analog input • PC5')]
for i,(a,b) in enumerate(bot):
 x=40+(i%4)*645;y=1390+(i//4)*125;tx(x,y,a,29,bold=True);tx(x,y+44,b,25)
tx(40,1660,'PWM: power outputs ≠ TTL; endstop/runout inputs ≠ clean PWM; motor coils ≠ signal supplies.',28,'#a22626',True)
tx(40,1710,'Reference photo: ender3.club • V4.2.2 silk checked • full electrical details, caveats and sources accompany this image.',23)
svg.append('</g></svg>');(P/'v422-all-connectors.svg').write_text(''.join(svg));cairosvg.svg2png(url=str(P/'v422-all-connectors.svg'),write_to=str(P/'v422-all-connectors.png'),output_width=3200,output_height=2142)
# Print-ready PDF with complete inventory. Portrait pages carry comfortable 9.3 pt body text.
c=canvas.Canvas(str(P/'v422-all-connectors-reference.pdf'),pagesize=landscape(A4));W,H=landscape(A4)
c.setTitle('Creality V4.2.2 - Complete Connector and PWM Reference')
c.setAuthor('Grok Bot - independent hardware research')
c.drawImage(str(P/'v422-all-connectors.png'),12,18,width=W-24,height=(W-24)*1740/2600);c.showPage()
body=ParagraphStyle('body',fontName='Helvetica',fontSize=9.3,leading=12.3,textColor=colors.HexColor('#20313d'))
small=ParagraphStyle('small',parent=body,fontSize=8.5,leading=11)
def para(txt,x,y,w,style=body):
 p=Paragraph(txt,style);_,h=p.wrap(w,1000);p.drawOn(c,x,y-h);return y-h
page=1
def start(title,sub):
 global page
 page+=1;c.setPageSize(A4);W,H=A4;c.setFillColor(colors.HexColor('#123e5c'));c.setFont('Helvetica-Bold',18);c.drawString(36,H-42,title)
 y=para(sub,36,H-58,W-72,small);return y-18

def foot():
 c.setFillColor(colors.grey);c.setFont('Helvetica',8);c.drawString(36,25,'V4.2.2 reference • functional nets only; no physical contact order assumed');c.drawRightString(A4[0]-36,25,str(page));c.showPage()

def card(r,y):
 x=36;w=A4[0]-72
 c.setFillColor(colors.HexColor('#e8f0f5'));c.roundRect(x,y-23,w,24,4,fill=1,stroke=0);c.setFillColor(colors.HexColor('#123e5c'));c.setFont('Helvetica-Bold',10.5);c.drawString(x+7,y-16,f"{r['id']}  {r['identifier']}  |  {r['contacts']}")
 y-=30
 lines=[f"<b>Function / electrical:</b> {e(r['function'])}. {e(r['electrical'])}.",f"<b>MCU:</b> {e(r['mcu'])}. <b>PWM:</b> {e(r['pwm'])}.",f"<b>Limits / evidence:</b> {e(r['notes'])} <font color='#667888'>[{e(r['sources'])}]</font>"]
 for line in lines:y=para(line,x+5,y,w-10)
 return y-13

groups=[('Power and fan connections','Fixed supply and low-side-switched 24 V outputs are electrically different from logic PWM.',R[:6]),('Motor and endstop connectors','Four motor sockets plus three TWO-PIN endstop inputs. Motor coils are never TTL signal contacts.',R[6:13]),('Runout, probe and temperature inputs','The working 3D Touch stays connected. Sensor-supply discrepancies and physical orientation remain explicit.',R[13:17]),('Display, storage and service interfaces','Unpopulated service pads are labelled as pads, not imaginary fitted sockets.',R[17:])]
for title,sub,rows in groups:
 y=start(title,sub)
 for r in rows:
  if y<180:
   foot();y=start(title+' (continued)',sub)
  y=card(r,y)
 if title.startswith('Runout'):
  y=para('<b>Laser-specific conclusion:</b> This document does not approve a direct Snapmaker connection to any socket. The Snapmaker\'s reported active-low, approximately 4 V control interface still needs a verified interface design. The Z socket divider/filter and the heater switched-return bypass problem are not solved by changing connector names.',36,y-5,A4[0]-72)
 foot()
y=start('Evidence, confidence and audit','Read together with the numbered photograph and the complete Markdown/CSV inventory.')
for title,txt in [('Scope','STM32F103RET6-based Ender-3 V2 V4.2.2 reference, visibly revision-checked. Component, driver and fan-fuse variants exist. This is not a factory schematic and not a fresh photograph of Michael\'s individual board.'),('Inventory audit','20 connector/terminal interfaces plus two unpopulated service-pad groups are identified. The fixed FAN screw pair is separate from both K-FAN sockets. X/Y/Z endstops are two-pin. No separate fitted reset switch or external I2C socket was identified. The EEPROM, fuses, LEDs, ICs, resistor networks and crystals are components, not additional connectors.'),('Confidence','High: physical connector identities/counts and common MCU net assignments. Reference-level: resistor/capacitor values, rail routing and individual variant matching. J1 supply remains unmeasured. Capacitors marked 100 nF inherit the schematic author\'s assumption, not a measurement.'),('Pin numbering','All 01-22 labels are connector callout IDs, NOT contact numbers. No physical left/right polarity is inferred from a schematic symbol. Probe silk is reproduced as a name only, not a new wiring instruction.'),('Firmware distinction','Hardware capability is not the currently configured output. Servo pulses, software fan PWM, heater power switching, timer alternate functions and stepper-driver chopping are separate classifications. SCOPE6 observations and Z measurements are supplied context, not newly verified hardware tests.')]:
 y=para(f'<b>{title}:</b> {e(txt)}',36,y,A4[0]-72);y-=12
for a,b,u in S:
 if y<105:foot();y=start('Sources (continued)','Exact source URLs; community versus manufacturer status is stated.')
 y=para(f'<b>{a}</b> — {e(b)}<br/><font size="8">{e(u)}</font>',36,y,A4[0]-72,small);y-=11
foot();c.save()
print('Wrote',len(R),'inventory entries')
