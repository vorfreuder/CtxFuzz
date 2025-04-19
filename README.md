![CtxFuzz](https://socialify.git.ci/vorfreuder/CtxFuzz/image?custom_description=discover+heap-based+memory+vulnerabilities&description=1&font=Source+Code+Pro&language=1&logo=https%3A%2F%2Fgithub.com%2Fvorfreuder%2FCtxFuzz%2Fblob%2Fmain%2Favatar.png%3Fraw%3Dtrue&name=1&owner=1&pattern=Floating+Cogs&theme=Dark)

![Static Badge](https://img.shields.io/badge/AFL++-%252300A98F?style=for-the-badge&logo=vowpalwabbit&logoColor=%2523FF9A00&label=fuzzers%20based)
![OS](https://img.shields.io/badge/OS-Linux-%23FCC624?style=for-the-badge&logo=linux)
![GitHub repo size](https://img.shields.io/github/repo-size/vorfreuder/fuzzdeploy?style=for-the-badge&logo=republicofgamers)

#### Introduction
___[CtxFuzz](https://sites.google.com/view/ctxfuzz/about) utilizes context heap operation sequences (the sequences of heap operations such as allocation, deallocation, read, and write that are associated with corresponding heap memory addresses) as a new feedback mechanism to guide the fuzzing process to explore more heap states and trigger more heap-based memory vulnerabilities.___

The implementation of CtxFuzz is under the [`fuzzers/ctxfuzz/repo`](fuzzers/ctxfuzz/repo) directory.

#### Evaluation
```bash
#!/bin/bash
set -e
apt-get update
apt-get install -y sudo tmux htop git curl python3 python3-pip python3-venv rsync
# install python3 packages
# export PIP_INDEX_URL=https://mirrors.aliyun.com/pypi/simple
pip3 install black isort numpy pandas styleframe docker
# install docker if you need
sudo apt-get update \
    && sudo apt-get install -y ca-certificates curl gnupg \
    && sudo install -m 0755 -d /etc/apt/keyrings \
    && curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg \
    && sudo chmod a+r /etc/apt/keyrings/docker.gpg \
    && echo \
    "deb [arch="$(dpkg --print-architecture)" signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
    "$(. /etc/os-release && echo "$VERSION_CODENAME")" stable" | \
    sudo tee /etc/apt/sources.list.d/docker.list > /dev/null \
    && sudo apt-get update \
    && sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
# sudo usermod -aG docker $USER
echo core | sudo tee /proc/sys/kernel/core_pattern
sudo bash -c 'cd /sys/devices/system/cpu; echo performance | tee cpu*/cpufreq/scaling_governor'

# Fuzzing
python3 evaluation.py
```
