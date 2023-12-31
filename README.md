<!--
\cond INTERNAL
-->
<div align="center" style="margin-bottom: 10px;">
<a href="https://github.com/raultapia/openev">
<img src="images/logo.png" alt="logo">
</a>
<p style="font-size: 30px" class="name">OpenEV</p>
</div>
<p align="center" class="brief">
Extending OpenCV to event-based vision
</p>
<!--
\endcond
-->

> **Disclaimer** *Hi there! This library is currently under construction. I will be releasing new features as soon as they are ready.*

## ⚙️ Installation

OpenEV uses `libcaer` library by [iniVation](https://inivation.com/) to configure and get data from event cameras. Click [here](https://gitlab.com/inivation/dv/libcaer) for more information.

Hence, installation of `libcaer` is required:
```bash
sudo add-apt-repository ppa:inivation-ppa/inivation
sudo apt update
sudo apt-get install libcaer-dev
```

Then, `openev` can be installed:
```bash
git clone https://github.com/raultapia/openev
mkdir -p openev/build
cd openev/build
cmake ..
make
sudo make install
```

## 📚 Documentation
The OpenEV documentation can be found [here](https://raultapia.github.io/openev).

## 🖥️ Usage
See [`examples`](https://github.com/raultapia/openev/tree/main/examples) folder.

## 📝 License

Distributed under the GPLv3 License. See [`LICENSE`](https://github.com/raultapia/openev/tree/main/LICENSE) for more information.

## 📬 Contact

[Raul Tapia](https://raultapia.com) - raultapia@us.es
