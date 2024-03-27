$module_name "program"
$export entry
$export frame

system_name: "system"
system: #0

graphics_name: "graphics"
graphics: #0


load_system: &system_name load_module &system ! ;
load_graphics: &graphics_name load_module &graphics ! ;

print:              &system     @ 0 extern_call ;
set_display_buf:    &graphics   @ 0 extern_call ;
iskeydown?:         &graphics   @ 1 extern_call ;
blit:               &graphics   @ 2 extern_call ;

sprite:
#b8 #b8 #[
00 00 0F 0F 0F 0F 00 00 
00 0F 00 00 00 00 0F 00 
0F 00 0F 00 00 0F 00 0F 
0F 00 00 00 00 00 00 0F 
0F 00 0F 00 00 0F 00 0F 
0F 00 00 0F 0F 00 00 0F 
00 0F 00 00 00 00 0F 00 
00 00 0F 0F 0F 0F 00 00 
]

drawpix: (x y --) 256 * + &screen + 15 swap !b ;
clear: 0 8192 $for [ 0 &screen r@ 2 * + ! ] ;


++!: dup @ inc swap ! ;
--!: dup @ dec swap ! ;

x: #10
y: #10
newx: #0
newy: #0

xbound: 256 8 - ;
ybound: 64 8 - ;

dec_ifnot0: dup 0 > $if [ dec ] ;

@pixel: &screen + @b ;
!pixel: &screen + !b ;

fade: 0 16384 $for [ r@ @pixel dec_ifnot0 r@ !pixel ] ;

between?: (value lower_inclusive upper_exclusive -- between?)
    2 pick <= $ifelse [ drop drop 0 ] [ >= ] ;

framenum: #0

frame:
    (clear)

    &framenum ++!
    &framenum @ 10 % 0 == $if [ fade ]

    &x @ &newx !
    &y @ &newy !
    (w) 87 iskeydown? $if [ &newy --! ]
    (a) 65 iskeydown? $if [ &newx --! ]
    (s) 83 iskeydown? $if [ &newy ++! ]
    (d) 68 iskeydown? $if [ &newx ++! ]
    (space) 32 iskeydown? $if [ clear ]

    &newx @ 0 xbound between? $if [ &newx @ &x ! ]
    &newy @ 0 ybound between? $if [ &newy @ &y ! ]

    &x @ &y @ &sprite blit
;

entry:
    load_system
    load_graphics
    &screen set_display_buf
;

screen:
$zeros #16384
