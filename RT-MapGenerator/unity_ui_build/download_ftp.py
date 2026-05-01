import os
import argparse
from ftplib import FTP, error_perm
import shutil
import tarfile

def download_archive_from_ftp(ftp_server, username, password, remote_path, local_path):

    # Create local directory if it doesn't exist
    if not os.path.exists(local_path):
        os.makedirs(local_path)

    # Connect to the FTP server
    ftp = FTP(ftp_server)
    ftp.login(user=username, passwd=password)

    # Set passive mode (optional but may help)
    ftp.set_pasv(True)

    # Recursively download files
    download_archive(ftp, remote_path, local_path)

    # Close FTP connection
    ftp.quit()

def download_archive(ftp, remote_path, local_path):
    # Ensure the remote path exists
    try:
        ftp.cwd(remote_path)
    except error_perm as e:
        print(f"Error: {e}")
        return

    # List the contents of the remote directory
    items = ftp.nlst()

    # Filter to include only files (exclude directories)
    files = []
    for item in items:
        try:
        	# Try to change into the item, If it worked, it's a directory — skip it
            ftp.cwd(f"{remote_path}/{item}")
            ftp.cwd(remote_path)
        except error_perm:
            files.append(item)

    if not files:
        print("No files found in the FTP directory.")
        return

    # Sort files assuming YYYY-MM-DD-style names and pick the most recent file
    files.sort(reverse=True)
    latest_file = files[0]

    item_remote_path = f"{remote_path}/{item}"
    item_local_path = os.path.join(local_path, item)

    if not os.path.exists(item_local_path):
        shutil.rmtree(local_path)
        os.makedirs(local_path)
        try:
            with open(item_local_path, 'wb') as local_file:
                ftp.retrbinary(f"RETR {item_remote_path}", local_file.write)
            os.chdir(local_path)
            tar = tarfile.open(item_local_path)
            tar.extractall(filter='fully_trusted')
            tar.close()
            os.remove(item_local_path)
        except Exception as e:
            print(f"Error downloading {item_remote_path}: {e}")

if __name__ == '__main__':
    # Set up the argument parser
    parser = argparse.ArgumentParser(description="A script that downloads a folder from FTP.")
    parser.add_argument("destination_path", type=str, help="The destination path")

    # Parse the arguments
    args = parser.parse_args()

    # Customize with your FTP server details
    ftp_server = 'h2322566.stratoserver.net'
    username = 'greendinoftp'
    password = '&j2#GTLGzkH!'
    remote_path = '/ftp/files/AutowareUnityBuild'

    local_path = os.path.join(args.destination_path, 'unity_build')

    download_archive_from_ftp(ftp_server, username, password, remote_path, local_path)
      