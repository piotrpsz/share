#include "share.h"
#include <QGuiApplication>
#include <QScreen>

i64 share::user_id_{1};

void share:: resize_and_center(QWidget *const widget, int const width, int const height) noexcept {
    auto const screen_geometry = QGuiApplication::primaryScreen()->geometry();
    auto const screen_width = screen_geometry.width();
    auto const screen_height = screen_geometry.height();
    auto const width_in_percent = width / 100.;
    auto const height_in_percent = height / 100.;
    auto const dx = int(screen_width * width_in_percent);
    auto const dy = int(screen_height * height_in_percent);
    widget->resize(dx, dy);

    auto const x = (screen_width - dx) / 2;
    auto const y = (screen_height - dy) / 2;
    widget->move(x, y);
}
