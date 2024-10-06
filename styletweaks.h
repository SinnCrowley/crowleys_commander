#ifndef STYLETWEAKS_H
#define STYLETWEAKS_H

#include <QProxyStyle>
#include <QStyledItemDelegate>


class StyleTweaks : public QProxyStyle {
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                           QPainter *painter, const QWidget *widget) const;

};

class EditRectangleDelegate : public QStyledItemDelegate {
public:
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};

#endif // STYLETWEAKS_H
