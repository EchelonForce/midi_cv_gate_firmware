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


def test():
    ports = mido.get_output_names()

    choice = input(f"{list_ports()}")
    idx = int(choice) - 1
    print(f"Choice: {ports[idx]}")

    choice = input(f"Channel (1-16): ")
    channel = int(choice)
    print(f"Channel: {channel}")

    with mido.open_output(ports[idx]) as port:
        for i in range(0, 2):
            for note in range(50, 66, 1):
                port.send(mido.Message("note_on", channel=channel, note=note))
                time.sleep(0.25)
            time.sleep(1.0)
            for note in range(50, 66, 1):
                port.send(mido.Message("note_off", channel=channel, note=note))
                time.sleep(0.25)
            time.sleep(1.0)


if __name__ == "__main__":
    test()
