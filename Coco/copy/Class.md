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