"""Package the committed source and a tested x64 build without local user data."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
TAG = 'v1.8.6.2-localfix.1'


def git(*args: str) -> bytes:
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'Bin/x64/Release')
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'release-artifacts')
    args = parser.parse_args()
    built = args.build_dir.resolve()
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if git('status', '--porcelain', '--untracked-files=no').strip():
        raise RuntimeError('Commit tracked changes before packaging a release.')
    commit = git('rev-parse', 'HEAD').decode().strip()
    tracked = [name for name in git('ls-files', '-z').decode('utf-8').split('\0') if name]
    names = ['TrafficMonitor.exe', 'TrafficMonitor.exe.config', 'OpenHardwareMonitorApi.dll', 'LibreHardwareMonitorLib.dll']
    names += [p.name for p in sorted((ROOT / 'OpenHardwareMonitorApi/runtime').glob('*.dll'))]
    for name in names:
        if not (built / name).is_file():
            raise FileNotFoundError(built / name)
    portable = output / f'TrafficMonitor-LocalFix-{TAG}-windows-x64.zip'
    source_zip = output / f'TrafficMonitor-LocalFix-{TAG}-source.zip'

    with tempfile.TemporaryDirectory(prefix='trafficmonitor-package-', dir=output) as temporary:
        app = Path(temporary) / 'TrafficMonitor-LocalFix'
        app.mkdir()
        for name in names:
            shutil.copy2(built / name, app / name)
        if sha(app / 'LibreHardwareMonitorLib.dll') != sha(ROOT / 'OpenHardwareMonitorApi/LibreHardwareMonitorLib.dll'):
            raise RuntimeError('Built hardware library differs from the committed dependency.')
        for runtime in (ROOT / 'OpenHardwareMonitorApi/runtime').glob('*.dll'):
            if sha(app / runtime.name) != sha(runtime):
                raise RuntimeError(f'Runtime dependency mismatch: {runtime.name}')
        for name in tracked:
            if name.startswith(('TrafficMonitor/language/', 'TrafficMonitor/skins/', 'TrafficMonitor/Logo/')):
                relative = Path(name).relative_to('TrafficMonitor')
                target = app / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / name, target)
        for name in ['README.md', 'README_en-us.md', 'LOCAL_FIX.md', 'LICENSE', 'LICENSE_CN']:
            shutil.copy2(ROOT / name, app / name)
        shutil.copytree(ROOT / 'licenses', app / 'licenses')
        (app / 'config.ini').write_text(
            '[general]\ncheck_update_when_start = false\nhardware_monitor_item = 3\n'
            'cpu_core_name = Core Average\nmonitor_time_span = 1000\n'
            '[config]\nshow_notify_icon = true\nshow_task_bar_wnd = true\nhide_main_window = 1\n'
            '[connection]\nauto_select = true\nselect_all = false\nconnection_name =\n'
            '[task_bar]\ntbar_display_item = 111\n', encoding='utf-8-sig')
        (app / 'global_cfg.ini').write_text('[config]\nportable_mode = true\n', encoding='utf-8-sig')
        if list(app.rglob('*.sys')) or list(app.rglob('*.dat')) or list(app.rglob('*.log')):
            raise RuntimeError('Driver or user data found in release staging.')
        binaries = [{'file': name, 'sha256': sha(app / name)} for name in names]
        with zipfile.ZipFile(portable, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
            for path in sorted(app.rglob('*')):
                if path.is_file():
                    archive.write(path, 'TrafficMonitor-LocalFix/' + path.relative_to(app).as_posix())

    subprocess.run(['git', '-C', str(ROOT), 'archive', '--format=zip',
                    '--prefix=TrafficMonitor-LocalFix-source/', '--output=' + str(source_zip), 'HEAD'], check=True)
    manifest = {
        'release_tag': TAG, 'file_version': '1.8.6.2', 'source_commit': commit,
        'upstream_repository': 'https://github.com/zhongyang219/TrafficMonitor',
        'upstream_commit': '930f17533d6098989ebad62f210aa97d75ef174b',
        'hardware_library_version': '0.9.6',
        'tested_platform': 'Windows 10 x64',
        'tested_hardware': ['Intel i5-10400', 'NVIDIA RTX 3060'],
        'tested_pawnio_version': '2.2.0',
        'cpu_temperature_test': {'samples': 15, 'minimum_celsius': 46.8, 'maximum_celsius': 48.3},
        'gpu_temperature_test': {'samples': 15, 'minimum_celsius': 50, 'maximum_celsius': 51},
        'taskbar_temperatures_verified': True,
        'build_toolset': 'VS2019 v142',
        'compatibility_defines': ['DISABLE_WINDOWS_WEB_EXPERIENCE_DETECTOR'],
        'full_taskbar_ui_verified': False, 'windows_11_verified': False,
        'bundled_sys_files': 0, 'configuration': 'generic defaults',
        'binaries': binaries,
        'archives': [{'file': p.name, 'sha256': sha(p)} for p in [portable, source_zip]],
    }
    manifest_path = output / 'build-info.json'
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    (output / 'SHA256SUMS.txt').write_text(
        ''.join(f'{sha(p)}  {p.name}\n' for p in [portable, source_zip, manifest_path]), encoding='utf-8')
    for path in [portable, source_zip]:
        with zipfile.ZipFile(path) as archive:
            if archive.testzip() is not None:
                raise RuntimeError(f'Archive integrity failure: {path.name}')
        print(f'Verified {path.name}: {path.stat().st_size} bytes')
    print(f'Packaged committed source {commit}')


if __name__ == '__main__':
    main()
