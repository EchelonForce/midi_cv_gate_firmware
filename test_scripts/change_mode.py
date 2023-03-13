import mido
import sys
import time


def list_ports():
    ports = mido.get_output_names()
    s = "Available MIDI Interfaces:\n"
    idx = 0
    for p in ports:
        idx += 1
        s += f"{idx} : {p}\n"

    s += "\nChoose an interface by num:\n"
    return s


def mode_list():
    s = "0 = FIRST_PRIO_POLY\n"
    s += "1 = 32_GATES\n"
    s += "2 = FIRST_PRIO_POLY_QUAD_HARMONIC\n"
    s += "3 = TEST_MODE\n"
    return s


def test():
    ports = mido.get_output_names()

    choice = input(f"{list_ports()}")
    idx = int(choice) - 1
    print(f"Choice: {ports[idx]}")

    choice = input(f"Channel (1-16): ")
    channel = int(choice)
    print(f"Channel: {channel}")

    choice = input(f"{mode_list()}\nMode: ")
    mode = int(choice)
    print(f"Mode: {mode}")

    DataEntryLSB = 38

    with mido.open_output(ports[idx]) as port:
        port.send(
            mido.Message(
                "control_change",
                channel=(channel - 1),
                control=DataEntryLSB,
                value=mode,
            )
        )


if __name__ == "__main__":
    test()
