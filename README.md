# Cursor Master

Cursor Master is an Unreal Engine 5 plugin for importing, organizing, and applying native hardware cursors from PNG files.

## Features

- Import square PNG cursors up to 256 x 256 pixels.
- Set a target size and hotspot during import.
- Store multiple cursor sizes in a `Hardware Cursor Asset`.
- Map Unreal cursor types in a `Hardware Cursor Collection Asset`.
- Apply a collection at runtime with Blueprint or C++.

## Installation

Copy the `CursorMaster` folder into your project's `Plugins` directory, then enable **CursorMaster** in Unreal Engine.

## Usage


1. Create a **Hardware Cursor Asset** in the Content Browser.
  <img width="468" height="128" alt="image" src="https://github.com/user-attachments/assets/fa1593b3-0f38-4549-b08e-de5b931ef7f8" />

2. Open it and import one or more PNG cursor images.
  <img width="476" height="470" alt="image" src="https://github.com/user-attachments/assets/446ce858-8053-46ac-8a4c-646c49535f7e" />
  <img width="369" height="246" alt="image" src="https://github.com/user-attachments/assets/6590d647-3705-45e7-acbe-28e76e948ebc" />

3. Create a **Hardware Cursor Collection Asset** and map each cursor type to a cursor asset.
  <img width="453" height="329" alt="image" src="https://github.com/user-attachments/assets/f2424d27-c9c8-4dcb-a194-a63f3f54af7d" />

5. Call `Apply(Size, OutError)` on the collection at runtime.

`Apply` selects the best available size for each cursor type and returns an error if hardware cursors are unsupported on the current platform.

## License

[MIT](LICENSE)
