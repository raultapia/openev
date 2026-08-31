<!-- \internal -->
<div align="center" style="margin-bottom: 10px;">
<a href="https://github.com/raultapia/openev">
<img src="https://github.com/raultapia/openev/blob/main/.github/assets/logo.png?raw=true" alt="openev">
</a>
</div>
<p align="center">
Extending OpenCV to event-based vision
</p>
<!-- \endinternal -->

> **Disclaimer** _Hi there! This library is currently under construction. I will be releasing new features as soon as they are ready._

## ⚙️ Installation

```bash
git clone https://github.com/raultapia/openev
mkdir -p openev/build
cd openev/build
cmake ..
make
sudo make install
```

## 🧩 Modules

OpenEV is modular. Every directory under [`modules`](https://github.com/raultapia/openev/tree/main/modules) is a module built as its own shared library, and all of them are built by default. Modules only depend on each other through headers, as shown below.

```
core ──┬── containers ──┬── devices
       │                ├── evproc
       │                └── readers
       └── representations

algorithms
```

| Module | Depends on | External dependencies |
| --- | --- | --- |
| `core` | - | OpenCV |
| `algorithms` | - | OpenCV, eFFT, Eigen |
| `containers` | `core` | OpenCV, Boost |
| `representations` | `core` | OpenCV, OpenCV viz |
| `devices` | `core`, `containers` | OpenCV, libcaer |
| `evproc` | `core`, `containers` | OpenCV |
| `readers` | `core`, `containers` | OpenCV, HDF5, pthread |

To leave a module out, set its option to `OFF`. The option name is the directory name in upper case.

```bash
cmake .. -DBUILD_MODULE_READERS=OFF
cmake .. -DBUILD_MODULE_DEVICES=OFF -DBUILD_MODULE_ALGORITHMS=OFF
```

## 🧪 Tests and benchmarks

Tests are only built in `Debug`.

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
ctest
```

Benchmarks are opt-in and independent of the build type, so that they can be measured on the same code that is shipped. Build them in `Release`.

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
make
make benchmarks
```

## 📚 Documentation

The OpenEV documentation can be found [here](https://raultapia.github.io/openev).

## 🖥️ Usage

See [`examples`](https://github.com/raultapia/openev/tree/main/examples) folder.

## 📝 License

Distributed under the GPLv3 License. See [`LICENSE`](https://github.com/raultapia/openev/tree/main/LICENSE) for more information.

## 📬 Contact

[Raul Tapia](https://raultapia.com) - raultapia@us.es
