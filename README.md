Emotionless
===========

Emotionless is a PS2 emulator with the intent to have clean, readable code that is both
portable and scalable, using Dolphin as an example of how to do this.

Currently, fixing the prime PS2 emulator would require a ridiculous amount of work.
If we're going to run on as many platforms as possible, it should be programmed from the ground up.

While developers behind PCSX2, Play! and hpsx64 have admirable projects,
I'm looking more towards creating something that to preserve the PS2 for the generations to come,
with well documented, easy to read, and clean code, hopefully the PS2 emulation scene
will benefit greatly from this project.

Tasks
================

- Get all EE opcodes implemented in the Interpreter.
- Create a scheduling system, one that's hopefully somewhat generic, like Dolphin's CoreTiming.
- Emulate the EE's TLB. I haven't found docs yet, so we might have to resort to docs on base MIPS or PCSX2 code.
- Create a basic Qt UI
- Create a basic Qt debugger

Goals
=====

- Clean code
- As small an amount of hacks and kludges as possible
- Accuracy as long it is still playable on modern computers.

Contribution guide
==================

- Write code
- Run astyle over changes
- PR code
