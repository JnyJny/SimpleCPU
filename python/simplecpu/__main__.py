"""
"""


from dataclasses import dataclass
from pathlib import Path
from sys import stdout, stderr

import typer

from loguru import logger

from .asm import Assembler
from .cpu import CPU
from .dis import Disassembler
from .memory import Memory
from .exceptions import ObjectFormatError


cli = typer.Typer()


@dataclass
class GlobalOptions:
    debug: bool = False


@cli.callback()
def main_callback(
    ctx: typer.Context,
    debug: bool = typer.Option(False, "--debug", "-d", is_flag=True),
) -> None:
    """A SimpleCPU Simulator"""

    (logger.enable if debug else logger.disable)("simplecpu")
    ctx.ensure_object(GlobalOptions)
    ctx.obj.debug = debug

    logger.remove()
    logger.add(
        stdout,
        format="<green>{time}</green>:<level>{level:>8}</level>:{message}",
        colorize=True,
        level="TRACE",
    )


@cli.command(name="run")
def load_and_run(
    ctx: typer.Context,
    path: Path,
    timer_interval: int = typer.Option(
        100,
        "--timer-interval",
        "-t",
        help="Duration in cycles between timer interrupts",
        show_default=True,
    ),
) -> None:
    """CPU Simulator"""

    try:
        memory = Memory.from_file(path)
    except ObjectFormatError:
        memory = Assembler.from_file(path).memory

    try:
        cpu = CPU(memory, timer_interval=timer_interval, debug=ctx.obj.debug)
    except Exception as error:
        print(error)
        raise typer.Exit(code=1) from None

    try:
        cpu.start()
    except Exception as error:
        typer.secho(error, fg="red")
        raise typer.Exit(code=1) from None


@cli.command(name="asm")
def assemble_source(
    ctx: typer.Context,
    source: Path,
    dest: Path = typer.Option(None, "--output", "-o"),
) -> None:
    """Assembler"""

    dest = dest or source.with_suffix(".o").relative_to(source.parent)

    try:
        assembler = Assembler.from_file(source)
        assembler.save(dest)
    except FileNotFoundError as error:
        typer.secho(f"{error.strerror}: '{error.filename}'", fg="red")
        raise typer.Exit(code=1)


@cli.command(name="dis")
def disassemble_source(
    ctx: typer.Context,
    objectpath: Path,
) -> None:
    """Disassembler"""

    try:
        disassembler = Disassembler(objectpath)
        print(disassembler)
    except FileNotFoundError as error:
        typer.secho(f"{error.strerror}: '{error.filename}'", fg="red")
        raise typer.Exit(code=1)
