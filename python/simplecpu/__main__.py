"""
"""


from pathlib import Path

import typer

from loguru import logger

from .asm import Assembler
from .cpu import CPU
from .dis import Disassembler
from .memory import Memory

loader = typer.Typer()
asm = typer.Typer()
dis = typer.Typer()


@loader.command()
def load_and_run(
    ctx: typer.Context,
    objectpath: Path,
    debug: int = typer.Option(False, "--debug", "-D", count=True),
    timer_interval: int = typer.Option(
        100,
        "--timer-interval",
        "-t",
        help="Duration in cycles between timer interrupts",
        show_default=True,
    ),
) -> None:
    """CPU Simulator"""

    for module in ["cpu", "memory"][:debug]:
        logger.enable(f"simplecpu.{module}")

    memory = Memory.from_file(objectpath)

    try:
        cpu = CPU(memory, timer_interval=timer_interval, debug=debug)
    except Exception as error:
        print(error)
        raise typer.Exit(code=1) from None

    try:
        cpu.start()
    except Exception as error:
        if debug:
            raise
        print(type(error), error)
        raise typer.Exit(code=1) from None


@asm.command()
def assemble_source(
    ctx: typer.Context,
    source: Path,
    dest: Path = typer.Option(None, "--output", "-o"),
    debug: bool = typer.Option(False, "--debug", "-D", is_flag=True),
) -> None:
    """Assembler"""

    (logger.enable if debug else logger.disable)("simplecpu.asm")

    dest = dest or source.with_suffix(".o").relative_to(source.parent)

    try:
        assembler = Assembler.from_file(source)
        assembler.save(dest)
    except FileNotFoundError as error:
        typer.secho(f"{error.strerror}: '{error.filename}'", fg="red")
        raise typer.Exit(code=1)


@dis.command()
def disassemble_source(
    ctx: typer.Context,
    objectpath: Path,
    debug: bool = typer.Option(False, "--debug", "-D", is_flag=True),
) -> None:
    """Disassembler"""

    (logger.enable if debug else logger.disable)("simplecpu.dis")

    try:
        disassembler = Disassembler(objectpath)
        print(disassembler)
    except FileNotFoundError as error:
        typer.secho(f"{error.strerror}: '{error.filename}'", fg="red")
        raise typer.Exit(code=1)
