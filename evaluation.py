from pathlib import Path
import fuzzdeploy

fuzzers = [
    "afl",
    "aflplusplus4_08c",
    "ctxfuzz",
    "htfuzz",
    "memlock",
    "tortoisefuzz",
]
targets = [
    "bento4",
    "cflow",
    "cxxfilt",
    "exiv2",
    "giflib",
    "mjs",
    "openh264",
    "yara",
    "yasm",
]
work_dir = Path("workdir")
while True:
    res = fuzzdeploy.build_images(
        fuzzers=fuzzers,
        targets=targets,
        log_path=work_dir / "logs",
        # skip_existed_fuzzer_images=False,
        # skip_existed_target_images=False,
    )
    for item in res:
        if item.code != 0:
            break
    else:
        break

fuzzdeploy.fuzzing(
    work_dir=work_dir,
    fuzzers=fuzzers,
    targets=targets,
    timeout="48h",
    repeat=8,
    # cpu_range=[i for i in range(40)],
)
