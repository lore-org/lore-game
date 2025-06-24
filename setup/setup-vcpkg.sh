. ~/.bashrc

if [ -z "$VCPKG_ROOT" ]; then
    echo '' >> ~/.bashrc
    echo '# setup-vcpkg' >> ~/.bashrc
    echo 'export VCPKG_ROOT=~/.vcpkg' >> ~/.bashrc
    echo 'export PATH=$PATH:$VCPKG_ROOT' >> ~/.bashrc

    . ~/.bashrc

    git clone https://github.com/microsoft/vcpkg "$VCPKG_ROOT"
fi

sh "$VCPKG_ROOT/scripts/bootstrap.sh" -disableMetrics