#pragma once

#include <QWidget>
#include <QStringList>

class QMouseEvent;
class QPaintEngine;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QWheelEvent;
class QString;

class OcctViewport : public QWidget
{
    Q_OBJECT

public:
    explicit OcctViewport(QWidget* parent = nullptr);
    ~OcctViewport() override;

    bool displayDemoPrism();
    bool exportCurrentShapeStep(const QString& path);

    QStringList availableNamedRegions() const;
    QString descriptionForNamedRegion(const QString& name) const;
    QString selectorRuleForNamedRegion(const QString& name) const;
    bool highlightNamedRegion(const QString& name);
    QString activeNamedRegion() const;

signals:
    void interactionStatus(const QString& message);
    void pickedNamedRegion(const QString& name);

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    QPaintEngine* paintEngine() const override;

private:
    bool initializeOcct();
    bool redisplayCurrentShape();
    QString namedRegionForFace() const;
    QString describeSelectedShape() const;

private:
    class Impl;
    Impl* m_impl;
};
