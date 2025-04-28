import os
import requests
import zipfile
from io import BytesIO

def download_github_repo(username, repo_name, output_dir=None):
    """
    Download a GitHub repository as a zip file and extract it
    
    Parameters:
    - username: GitHub username
    - repo_name: Repository name
    - output_dir: Directory to extract files to (default is current directory)
    """
    # Create output directory if it doesn't exist
    if output_dir is None:
        output_dir = os.getcwd()
    os.makedirs(output_dir, exist_ok=True)
    
    # Construct the URL for the repository zip file
    zip_url = f"https://github.com/{username}/{repo_name}/archive/refs/heads/main.zip"
    
    print(f"Downloading repository from {zip_url}...")
    
    try:
        # Download the zip file
        response = requests.get(zip_url)
        response.raise_for_status()  # Raise an exception for HTTP errors
        
        # Extract the zip file
        with zipfile.ZipFile(BytesIO(response.content)) as zip_ref:
            print(f"Extracting files to {output_dir}...")
            zip_ref.extractall(output_dir)
        
        # Find the extracted folder name
        extracted_folder = None
        for item in os.listdir(output_dir):
            if os.path.isdir(os.path.join(output_dir, item)) and item.startswith(f"{repo_name}-"):
                extracted_folder = item
                break
        
        if extracted_folder:
            print(f"Repository downloaded and extracted to {os.path.join(output_dir, extracted_folder)}")
            return os.path.join(output_dir, extracted_folder)
        else:
            print("Repository downloaded but couldn't identify the extracted folder")
            return output_dir
    
    except requests.exceptions.RequestException as e:
        print(f"Error downloading repository: {e}")
        return None

def list_dataset_files(repo_dir):
    """
    List all potential dataset files in the repository
    Focus on common dataset formats and image files
    """
    dataset_extensions = ['.csv', '.json', '.xlsx', '.xls', '.txt', '.png', '.jpg', '.jpeg', '.bmp']
    dataset_files = []
    
    # Walk through the repository directory
    for root, _, files in os.walk(repo_dir):
        for file in files:
            _, ext = os.path.splitext(file)
            if ext.lower() in dataset_extensions:
                dataset_files.append(os.path.join(root, file))
    
    # Print found dataset files
    if dataset_files:
        print("\nFound potential dataset files:")
        for file in dataset_files:
            print(f"- {os.path.relpath(file, repo_dir)}")
    else:
        print("\nNo dataset files found in the repository.")
    
    return dataset_files

if __name__ == "__main__":
    # Set your GitHub username and repository name
    username = "Sandra15-ux"
    repo_name = "Sandra_Tang_Poh_Yi-Saw_Hui_Lin_RSW3S2_G3"
    
    # Set output directory (default is current directory)
    output_dir = "downloaded_repo"
    
    # Download and extract the repository
    repo_dir = download_github_repo(username, repo_name, output_dir)
    
    # If download successful, list dataset files
    if repo_dir:
        dataset_files = list_dataset_files(repo_dir)
        
        print("\nRepository download complete!")
        print(f"Files have been extracted to: {repo_dir}")