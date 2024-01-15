FROM archlinux:base-devel

RUN pacman -Sy --needed --noconfirm git

RUN git clone --depth 1 https://github.com/aadilshabier/poppler-multithreaded.git

WORKDIR /poppler-multithreaded

RUN pacman -S --needed --noconfirm neovim cmake freetype2 fontconfig nss libjpeg-turbo openjpeg2 aws-sdk-cpp

RUN mkdir build

#RUN cmake -B ./build -S .
#RUN ninja pdftohtml

CMD ["bash"]