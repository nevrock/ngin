# This script automates the CMake build process in the current directory.

# Remove the existing 'build' directory if it exists.
# The 'rm -rf' command removes directories and their contents recursively and forcefully.
echo "Removing existing 'build' directory (if any)..."
rm -rf build

# Create a new 'build' directory.
echo "Creating new 'build' directory..."
mkdir build

# Change the current directory to 'build'.
echo "Changing directory to 'build'..."
cd build

# Run CMake to configure the project.
# 'cmake ..' tells CMake to look for the CMakeLists.txt file in the parent directory.
echo "Running CMake to configure the project..."
cmake ..

# Build the project using CMake's build command.
# 'cmake --build .' tells CMake to build the project in the current directory.
echo "Building the project..."
cmake --build .

echo "CMake build process completed."
