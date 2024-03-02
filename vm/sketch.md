# ideas

OOP-like header interface

```
.send word
.receive -- word
```

code:
```
module mylib
fn double ( word -- word )
    2 *
end

fn triple ( word -- word )
    3 *
end

fn getmagic ( -- word )
    42
end
```
- double is function 0
- triple is function 1
- getmagic is function 3

interface:
```
interface mylib
0 double ( word -- word )
1 triple ( word -- word )
2 getmagic ( -- word )
```

code:
```
.abcdef (push string abcdef on to stack)
.mylib.getmagic call

call (?(args) str(module) str(function) -- ?(returns))
```
nah none of that dynamic shit

most basic interface:
```
interface system
{
    0 print ( word )
    1 getkey ( -- word )
}
```

module format
```
0: pointer to first fn start byte
1: pointer to second fn start byte
```

control flow

if
if the top of the stack is nonzero, jump to .then, else jump to .else. jumps are always relative
```
1 if .then .else
:then
    1 1 +
:else
    2 2 +



( functions )
:my_add_one
    1 +
; (return)


.my_add_one call

(
    call: push current instruction pointer to return stack
    ;: jump to top instruction pointer on return stack
)
```

strings -> stored in data, load pointer
```
:system_module "system"
```

external module call
```
:systemname "system"
("string" add null terminated string here)
:system #0
(#n: add the literal 0 to program data here)

:entry
    .systemname loadmodule
    .system store
```



defining own external symbols
```
@fnname1
@fnname2
@fnname3
```