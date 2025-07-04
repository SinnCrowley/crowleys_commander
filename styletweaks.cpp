#include "styletweaks.h"

#include <QStyleOption>
#include <QPainter>

// selection cursor rectangle and drop indicator
void StyleTweaks::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                   QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_FrameFocusRect) {

        if (widget) {
            // don't draw focus rectangle for settings tabs
            if (widget->objectName() == "menuTabs")
                return;
        } else {
            qreal x = option->rect.x() + 1;
            qreal y = option->rect.y() + 1;
            qreal w = option->rect.width() - 1;
            qreal h = option->rect.height() - 2;

            painter->setPen(QColor(250, 82, 82));
            painter->drawRect(QRectF(x, y, w, h));
            return;
        }

        return;
    }

    if (element == QStyle::PE_IndicatorItemViewItemDrop) {
        QStyleOption opt(*option);
        opt.rect.setLeft(0);
        if (widget)
            opt.rect.setRight(widget->width());

        QProxyStyle::drawPrimitive(element, &opt, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
