# Class

```cpp
#pragma once

#include <QObject>
#include <QWidget>

#include "Coco/Debug.h"
#include "Coco/Layout.h"

class Class : public QWidget
{
    Q_OBJECT

public:
    explicit Class(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        initialize_();
    }

    virtual ~Class() override { COCO_TRACER; }

private:
    void initialize_()
    {
        //...
    }
};
```

```cpp
#pragma once

#include <QObject>
#include <QWidget>

#include "Coco/Debug.h"

class Class : public QWidget
{
    Q_OBJECT

public:
    using QWidget::QWidget;
    virtual ~Class() override { COCO_TRACER; }

};
```
