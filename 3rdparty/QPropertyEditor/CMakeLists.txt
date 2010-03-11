project(QPROPEDIT)

set(QPROP_SRCS
    ColorCombo.cpp
    Property.cpp
    QPropertyEditorWidget.cpp
    QPropertyModel.cpp
    QVariantDelegate.cpp
    EnumProperty.cpp
)

set(QPROP_HDRS
    ColorCombo.h
    Property.h
    QPropertyEditorWidget.h
    QPropertyModel.h
    QVariantDelegate.h
    EnumProperty.h
)

qt4_wrap_cpp(QPROP_MOC_SRCS ${QPROP_HDRS})
add_library(QPropertyEditor STATIC ${QPROP_SRCS} ${QPROP_MOC_SRCS} ${QPROP_HDRS})