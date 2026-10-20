# BeamMP-Launcher

The launcher is the way we communitcate to outside the game, it does a few automated actions such as but not limited to: downloading the mod, launching the game, and create a connection to a server.

## [Getting started](https://docs.beammp.com/game/getting-started/)

## License

BeamMP Launcher, a launcher for the BeamMP mod for BeamNG.drive
Copyright (C) 2024 BeamMP Ltd., BeamMP team and contributors.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Optional desktop frontend (community preview)

This branch includes a Qt 6 Widgets frontend with account sign-in, a Play button,
streamed console logs, and settings. The original console launcher remains the
backend and can still be built and used independently.

See [the desktop setup and build guide](docs/DESKTOP-FR.md). Build both programs
with `-DBEAMMP_BUILD_GUI=ON` and a Qt CMake prefix, or use the manual Windows
workflow. `-DBEAMMP_GUI_ONLY=ON` builds the frontend independently for UI work.
