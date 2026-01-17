import os
import sys
import json
import subprocess
import zipfile
import shutil
import argparse

# Configuration
SOURCE_RELEASE = "third-party-srcs"
ARTIFACT_RELEASE = "third-party-deps"
REPO = "jasonstredwick/llm"

print(f"Python Version: {sys.version}")

def run_cmd(cmd, cwd=None, env=None):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: Command failed with return code {result.returncode}")
        print(f"STDOUT: {result.stdout}")
        print(f"STDERR: {result.stderr}")
    return result.returncode

def check_gh_auth():
    return run_cmd(["gh", "auth", "status"]) == 0

def download_artifact(release, pattern, dest_dir, expected_file=None):
    os.makedirs(dest_dir, exist_ok=True)
    ret = run_cmd(["gh", "release", "download", release, "-R", REPO, "-p", pattern, "--dir", dest_dir, "--clobber"])
    if ret != 0:
        return False
    if expected_file:
        full_path = os.path.join(dest_dir, expected_file)
        if not os.path.exists(full_path):
            print(f"Error: Expected file {expected_file} was not downloaded.")
            return False
    return True

def upload_artifact(release, file_path):
    return run_cmd(["gh", "release", "upload", release, "-R", REPO, file_path, "--clobber"]) == 0

def get_installed_manifest(sysroot):
    manifest_path = os.path.join(sysroot, ".installed.json")
    if os.path.exists(manifest_path):
        with open(manifest_path, "r") as f:
            return json.load(f)
    return {"platform": None, "dependencies": {}}

def save_installed_manifest(sysroot, manifest):
    manifest_path = os.path.join(sysroot, ".installed.json")
    os.makedirs(sysroot, exist_ok=True)
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=4)

def build_dependency(name, version, platform, root, lock, current_sysroot):
    artifact_name = f"{name}-{version}_{platform}.zip"
    source_archive = f"{name}-{version}.zip"
    workspace = os.path.join(root, "third_party_workspace")

    # Isolate build and install
    lib_build_parent = os.path.join(workspace, "build", name)
    lib_install_dir = os.path.join(workspace, "install", name)
    os.makedirs(lib_build_parent, exist_ok=True)
    os.makedirs(lib_install_dir, exist_ok=True)

    print(f"Building {name}-{version} from source...")
    if not download_artifact(SOURCE_RELEASE, source_archive, workspace, expected_file=source_archive):
        print(f"Error: Source archive {source_archive} not found in {SOURCE_RELEASE}")
        return None

    # Extract source
    zip_path = os.path.join(workspace, source_archive)
    src_dir = os.path.join(lib_build_parent, f"{name}-{version}")
    # Adjust for cases like zlib131 vs zlib-1.3.1
    # We'll just extract to lib_build_parent and find the single folder
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(lib_build_parent)

    # Try to find the source dir within lib_build_parent
    # (some zips have a root folder, others don't)
    subdirs = [os.path.join(lib_build_parent, d) for d in os.listdir(lib_build_parent) if os.path.isdir(os.path.join(lib_build_parent, d))]
    if len(subdirs) == 1:
        extracted_src = subdirs[0]
    else:
        extracted_src = lib_build_parent # Hope for the best

    # Prepare build dir
    build_dir = os.path.join(extracted_src, "build")
    os.makedirs(build_dir, exist_ok=True)

    # Resolve args
    common_args = lock.get("common", {}).get(platform, [])
    dep_config = next((d for d in lock["platform"][platform] if d["name"] == name), {})
    specific_args = dep_config.get("cmake_args", [])

    all_args = common_args + specific_args
    resolved_args = []
    # For build phase, placeholders resolve to the *currently partially populated* sysroot
    for arg in all_args:
        if "{{" in arg:
            for d in lock["platform"][platform]:
                placeholder = "{{" + d["name"] + "_DIR}}"
                if placeholder in arg:
                    # Point to the sysroot where previous dependencies were just extracted
                    arg = arg.replace(placeholder, current_sysroot.replace("\\", "/"))
        resolved_args.append(arg)

    # Configure & Build
    cmake_cmd = ["cmake", "-S", extracted_src, "-B", build_dir] + resolved_args
    cmake_cmd.append(f"-DCMAKE_INSTALL_PREFIX={lib_install_dir.replace('\\', '/')}")

    if run_cmd(cmake_cmd) != 0: return None
    if run_cmd(["cmake", "--build", build_dir, "--config", "Release"]) != 0: return None
    if run_cmd(["cmake", "--install", build_dir, "--config", "Release", "--prefix", lib_install_dir.replace('\\', '/')]) != 0: return None

    # Package individual artifact
    artifact_zip = os.path.join(workspace, "artifacts", artifact_name)
    os.makedirs(os.path.dirname(artifact_zip), exist_ok=True)
    with zipfile.ZipFile(artifact_zip, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for r, ds, fs in os.walk(lib_install_dir):
            for f in fs:
                full_path = os.path.join(r, f)
                rel_path = os.path.relpath(full_path, lib_install_dir)
                zipf.write(full_path, rel_path)

    # Upload if authenticated
    if check_gh_auth():
        upload_artifact(ARTIFACT_RELEASE, artifact_zip)
    else:
        print(f"Warning: Skipping upload for {artifact_name} (not authenticated)")

    # Cleanup intermediate factory noise
    print(f"Cleaning up factory sandbox for {name}...")
    shutil.rmtree(lib_build_parent)
    shutil.rmtree(lib_install_dir)

    return artifact_zip

def sync(args):
    root = args.root
    platform = args.platform
    sysroot = os.path.join(root, "deps")
    temp_dir = os.path.join(root, "temp_download")

    with open(os.path.join(root, "deps.lock.json"), "r") as f:
        lock = json.load(f)

    if platform not in lock["platform"]:
        print(f"Error: No configuration for platform {platform}")
        sys.exit(1)

    lock_deps = {d["name"]: d["version"] for d in lock["platform"][platform]}
    installed_manifest = get_installed_manifest(sysroot)

    # Check if we need to sync
    needs_sync = (installed_manifest["platform"] != platform)
    if not needs_sync:
        for name, version in lock_deps.items():
            if installed_manifest["dependencies"].get(name) != version:
                needs_sync = True
                break

    if not needs_sync:
        # Extra safety check: does the include dir exist?
        if os.path.exists(os.path.join(sysroot, "include")):
            print("Dependencies are up to date.")
            return
        else:
            print("Missing sysroot contents, forcing sync...")

    print(f"Syncing dependencies for {platform}...")

    # Wipe old sysroot
    if os.path.exists(sysroot):
        shutil.rmtree(sysroot)
    os.makedirs(sysroot, exist_ok=True)

    new_manifest = {"platform": platform, "dependencies": {}}

    for name, version in lock_deps.items():
        artifact_name = f"{name}-{version}_{platform}.zip"

        # 1. Try download binary
        artifact_path = None
        if download_artifact(ARTIFACT_RELEASE, artifact_name, temp_dir, expected_file=artifact_name):
            artifact_path = os.path.join(temp_dir, artifact_name)
        else:
            # 2. Build from source
            # Note: We pass the sysroot so the build can find previously extracted dependencies
            artifact_path = build_dependency(name, version, platform, root, lock, sysroot)
            if not artifact_path:
                print(f"Error: Failed to acquire {name}-{version}")
                sys.exit(1)

        # 3. Extract to sysroot (Populate ONLY from ZIP)
        print(f"Installing {name}-{version} to sysroot...")
        with zipfile.ZipFile(artifact_path, 'r') as zip_ref:
            zip_ref.extractall(sysroot)

        new_manifest["dependencies"][name] = version

        # Cleanup temp artifact
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

    save_installed_manifest(sysroot, new_manifest)
    print("Sync complete.")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", required=True)
    parser.add_argument("--root", required=True)
    args = parser.parse_args()
    sync(args)

if __name__ == "__main__":
    main()
