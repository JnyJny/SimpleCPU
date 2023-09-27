"""
"""


from pathlib import Path

import typer

from loguru import logger

from .asm import Assembler
from .cpu import CPU
from .dis import Disassembler
from .memory import Memory
from .objectfile import ObjectFile

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

    for module in ["cpu", "memory", "objectfile"][:debug]:
        logger.enable(f"cpusim.{module}")

    objectfile = ObjectFile(objectpath)

    try:
        cpu = CPU(objectfile.image, timer_interval=timer_interval, debug=debug)
    except Exception as error:
        print(error)
        raise typer.Exit(code=1) from None

    try:
        cpu.start()
    except Exception as error:
        print(error)
        raise typer.Exit(code=1) from None


@asm.command()
def assemble_source(
    ctx: typer.Context,
    source: Path,
    dest: Path = typer.Option(None, "--output", "-o"),
    debug: bool = typer.Option(False, "--debug", "-D", is_flag=True),
) -> None:
    """Assembler"""

    (logger.enable if debug else logger.disable)("cpusim.asm")

    dest = dest or source.with_suffix(".o").relative_to(source.parent)

    try:
        assembler = Assembler(source)
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

    (logger.enable if debug else logger.disable)("cpusim.dis")

    try:
        disassembler = Disassembler(objectpath)
        print(disassembler)
    except FileNotFoundError as error:
        typer.secho(f"{error.strerror}: '{error.filename}'", fg="red")
        raise typer.Exit(code=1)
