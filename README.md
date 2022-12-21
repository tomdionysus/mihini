# Mihini

Mihini is virtual machine for embedded use.

*mihīni (loan) (noun) - machine, engine - [Te Aka](https://maoridictionary.co.nz/word/4064)*

*Status:* Mihini is alpha. Please don't use it in anything resembling production or rely on it - yet. 

## Components

* `mhvm` VM frontend (alpha)
* `mhasm` assembler (beta)
* `mhdasm` disassembler (beta)

* `libmihini` VM library (alpha)

## Building

Mihini has no dependencies other than `stdlib` (and cmake), so building the `mhasm`, `mhdasm` and `mihini` executables is trivial:

```
cmake .
make
```

## mihini VM

`mihini <bytecodefile>`

Execute the 

By default, the VM has 1Mb of memory and a 4kbyte stack.

## mhasm Assembler

`mhasm <assembly.mhasm> <outputbytecodefile>` 

## mhdasm Disassembler

`mhdasm <outputbytecodefile> <assembly.mhasm>` 

## mhvm Virtual Machine

`mhdasm <outputbytecodefile>` 

## VM Layout

See the [VM Layout Doucmentation](VM_LAYOUT.md).

## System Calls

See the [syscall Documentation](SYSTEM.md).

## Examples

See the [example](example) dir for `hello_world.mhasm` and further examples.

## Tests
The `mihini_tests` executable is built along with everything else into the `bin` dir. Execute the tests like so:

```
mihini_tests
```