#pragma once

#include "Theme.h"

#include <QString>
#include <QWidget>

// The display panel: a small line (preview or finished expression) above a
// large line (what you are typing, or the result). The large line shrinks to
// fit and, if it still does not fit, drops its oldest characters.
class DisplayWidget : public QWidget {
public:
    explicit DisplayWidget(const Theme& theme, QWidget* parent = nullptr);

    void setContent(const QString& primary, const QString& secondary, bool isError);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Theme theme_;
    QString primary_;
    QString secondary_;
    bool isError_ = false;
};