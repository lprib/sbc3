(
systemname: "system"
system: #0

mul2: 2 * ;

entry:
    &systemname load_module
    &system ! (todo: if a label is declared in data, default to pushing address
    rather than calling)
    &system @ 0 extern_call
)
3 4 <
if!
[ 3 + ]
[ 5 * ]