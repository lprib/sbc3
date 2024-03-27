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

setup_screen:
    &graphics_name load_module &graphics !
    &screen set_display_buf
;

entry: load_system setup_screen ;

drawpix: (x y --) 256 * + &screen + 15 swap !b ;
clear: 0 8192 $for [ 0 &screen r@ 2 * + ! ] ;

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

++!: dup @ inc swap ! ;
--!: dup @ dec swap ! ;

x: #10
y: #10

frame:
    clear

    (w) 87 iskeydown $if [&y --!][]
    (a) 65 iskeydown $if [&x --!][]
    (s) 83 iskeydown $if [&y ++!][]
    (d) 68 iskeydown $if [&x ++!][]

    &x @ &y @ &sprite blit
;

screen:
$zeros #16384
