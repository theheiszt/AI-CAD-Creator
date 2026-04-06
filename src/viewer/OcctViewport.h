#pragma once

#include <QWidget>
#include <QStringList>

class QPaintEngine;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
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

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    QPaintEngine* paintEngine() const override;

private:
    bool initializeOcct();
    bool redisplayCurrentShape();

private:
    class Impl;
    Impl* m_impl;
};
