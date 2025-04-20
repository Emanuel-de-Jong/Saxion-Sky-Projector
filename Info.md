## Teacher
Douwe van Twillert

## Links
https://nl.mouser.com/
https://www.conrad.nl/nl
https://www.kicad.org/
https://elektronicawereld.nl/

## Blender 3d model formats (in order of popularity and how nice it is to work with)
Filmbox (.fbx)
glTF 2.0 (.glb/.gltf)
Collada (.dae)
Wavefront (.obj)
Standford PLY (.ply)
Alembic (.abc)
STL (.stl)

## Keep in mind
star grounding scheme
upper floor holes in 2 rows for smaller steps
bunch of holes/slits in upper floor for air flow and easier wiring
air holes at lower floor
air holes above power input at fan blowing in
feet with rubber underneath
volt and amps engraved in shell

## Fan decibels
intel 8				45 very low
brushless 7			40,4 mid
akasa 5				35,6 low
top motor 4			33,4 mid

## Visual ideas
3 different directional
3 different global/circular
shared directional
shared global
shared directional and leds moving left and right
shared directional and leds moving left and right without disc
shared global and leds circling
shared global and leds circling without disc

## Distance LED to disc (cm)
3: 0,8
4: 0,95
5: 1,7
6: 1,1
7: 1,1
avg: 1,13
my pick: 1

## Distance disc to outer lens (disc to bottom of lens + bottom of lens to highest point of inner dome)
1: 0,6 + 2,35 = 2,95
2: 0,7 + 1,9 = 2,6
3: 0,8 + 3,65 = 4,45
4: 0,4 + 2,9 = 3,3
5: 2,75
6: 3,2
7: 3,4
avg: 3,24
my pick: 3,2

## To buy

## Measurements
Width/Length/Height/Thickness in CM

arduino 7/5,5/1,3
microswitch (without knob) 3,8/1/1,9
servo (with rotater) 4(5,5 with screw panels)/2/4,4
potmeter (without stick/wires up) 2/2/4
magnet 4,5/1,3/1,6
plate for magnet 4,5/1,2/0,1

dome
cylinder
top floor
bottom floor
card

## Further Info
A diy sky projector that is steered through an arduino r4 wifi. with 3 servos with each a 3w rgb led on them going left and right at different speeds. a microswitch to turn it on, a potmeter to change the global speed modifier of the servos, a potmeter to change the global brightness of the leds. The arduino gives each rgb led a different color and changes it every few ms for a fading rainbow effect. a 14 AWG barrel jack female input at the outside of the device to power the arduino, servos, leds and fans. All grounds are shared and the ground cables try to follow a star grounding scheme.

components:
adapter:
	Ingang: 100-240V 50-60Hz
	Uitgang: 12V DC 8A
	Connectoren: DC-DC-stekker 5,5 mm x 2,1 mm 
	Maximaal vermogen: 96W
buck converter:
	In: 12v
	Out: 5v
	Max ohm: 5
	pcb is heatsink
pwm driver PCA9685:
    Met I2C-communicatie, een geïntegreerde PWM-driver en klokzender hoef je niet voortdurend signalen te verzenden die je microcontroller belasten.
    Deze servodriver PCA9685 beschikt over een ompolingsbeveiliging en 220 ohm-weerstanden op elke uitgang voor eenvoudige bediening van servo's en leds
    5V-compatibiliteit betekent dat je ook een 3,3 VMCU kunt gebruiken om uitgangen tot 6 V te bedienen en veilig te bedienen.
    Er zijn slechts twee pinnen nodig om 16 vrijlopende PWM-uitgangen te bedienen, die werken met maximaal 6 V, en zelfs tot 992 PWM-uitgangen kunnen door de aansluiting van maximaal 62 kabels worden aangestuurd.
    12-bits resolutie voor elke uitgang. Ongeveer 4us resolutie bij 60 Hz vernieuwingssnelheid voor servo's
IRLZ44N mosfets:
	amount: 9
	Nominale spanning: 55 V; nominale stroom: 47 A; verliesvermogen: 110 W. 
	Polariteit: N-kanaaldrain source spanning: 55VDrain Source-weerstand: 22 mΩContinue stroom: 47 A. Bedrijfstemperatuur: -55 ℃ ~ 175 ℃ 
	RDS (on) (@10V)   max: 22 mΩ 
	RDS (on) (@4.5V)   max: 35 mΩ 
	VGS(th)   min  max: 1.5 V   1 V   2 V 
	VGS   max: 16 V 
S3003 RC Servos:
	amount: 3
rgb leds:
	amount: 9
	pins: 6 (3 anode, 3 cathode)
	Spanning: rood max. 2,4V, groen/blauw max. 3,4V
	Stroom: ongeveer 350mA
	Stroomverbruik: ongeveer 3W
	gesoldeerd op een aluminium printplaat
fans:
	amount: 2
	pins: 2
	power: 12v
microswitch
10k potmeter
capacitors
resistors

wiring:
1.
	240v ac european home power outlet connected to power adapter
	adapter end is barrel jack connected to female barrel jack cable glued to projector
	the other side of the glued cable is open to distribute to different sources in the projector
	a 1000µF 25v electrolytic capacitor and a 0.1µF 50V ceramic capacitor in parallel between the 5v and ground
	we'll call this the main power source (mps) from now on
2.
	2x fan connected to mps and common ground
	one is for general circulation, the other blows directly on the 12v to 5v converter
3.
	cable with barrel jack male on 1 side and loose cable on the other
	loose cable soldered to mps
	barrel jack goes into arduino (12v)
	arduino ground is connected to common ground
4.
	2x potmeter connected to arduino pin, arduino 5v and common ground
5.
	microswitch connected to arduino pin with internal INPUT_PULLUP and common ground
6.
	power cable soldered to mps and to 12v to 5v converter
	5v output goes to the 3 servos
	each servo has a 1000µF 25V electrolytic capacitor and 0.1µF 50v ceramic capacitor in parallel between it's 5v and ground
	3 pwm arduino outputs go to the 3 servos
7.
	IC connection between arduino and pwm driver
	power distribution board connected to mps (12v)
	3 output channels per rgb led (9 channels total)
	each red with 27ohm and 2,7ohm 5w chained resistor
	each green and blue with a 27ohm 5w resistor
	other ends of resistors go to the r, g and b anodes of the leds
	r, g and b cathodes of the leds go to their own mosfet, connecting to its drain
	each mosfet's gate connected to a channel on the pwm driver
	each mosfet's source connected to common ground
	resistors are mounted with thermal pads to a heatsink
