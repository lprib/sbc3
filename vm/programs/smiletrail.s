$module_name "program"
$export entry
$export frame

system_name: "system"
system: #0

graphics_name: "graphics"
graphics: #0


load_system: &system_name load_module &system ! ;

print:              &system     @ 0 extern_call ;
set_display_buf:    &graphics   @ 0 extern_call ;
iskeydown:          &graphics   @ 1 extern_call ;
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

setup_screen:
    &graphics_name load_module &graphics !
    &screen set_display_buf
;

entry: load_system setup_screen ;

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

dec_ifnot0:
    dup 0 > $if
        [ dec ]
        [ ]
;

fade:
    0 16384 $for [
        &screen r@ + @b
        dec_ifnot0
        &screen r@ + !b
    ]
;

framenum: #0

frame:
    (clear)

    &framenum ++!
    &framenum @ 10 % 0 == $if [
        fade
    ][]

    &x @ &newx !
    &y @ &newy !
    (w) 87 iskeydown $if [&newy --!][]
    (a) 65 iskeydown $if [&newx --!][]
    (s) 83 iskeydown $if [&newy ++!][]
    (d) 68 iskeydown $if [&newx ++!][]

    &newx @ xbound <= $if [
        &newx @ 0 >= $if [
            &newx @ &x !
        ][]
    ][]
    &newy @ ybound <= $if [
        &newy @ 0 >= $if [
                    &newy @ &y !
        ][]
    ][]

    &x @ &y @ &sprite blit
;

screen:
$zeros #16384
