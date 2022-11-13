#/usr/bin/env bash

main() {
  if [ ! command -v shpec &>/dev/null ]; then
    sh -c "`curl -L https://raw.githubusercontent.com/rylnd/shpec/master/install.sh`"
  fi

  if [ ! command -v entr &>/dev/null ]; then
    git clone https://github.com/eradman/entr.git /tmp/entr \
      && cd /tmp/entr \
      && ./configure \
      && make install \
      && cd
  fi

  echo "Test deps installed"
}

main
