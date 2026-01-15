"""
Flow Layout

A layout that arranges widgets in a flowing grid pattern.
Widgets wrap to the next row when they don't fit.
"""

from PyQt6.QtWidgets import QLayout, QLayoutItem, QSizePolicy, QWidget
from PyQt6.QtCore import Qt, QRect, QSize, QPoint


class FlowLayout(QLayout):
    """
    A layout that arranges child widgets horizontally, wrapping to new rows.
    """
    
    def __init__(self, parent: QWidget = None, margin: int = -1, 
                 h_spacing: int = -1, v_spacing: int = -1):
        super().__init__(parent)
        
        self._items: list[QLayoutItem] = []
        self._h_spacing = h_spacing
        self._v_spacing = v_spacing
        
        if margin >= 0:
            self.setContentsMargins(margin, margin, margin, margin)
    
    def addItem(self, item: QLayoutItem):
        self._items.append(item)
        self.invalidate()  # Force layout recalculation
    
    def horizontalSpacing(self) -> int:
        if self._h_spacing >= 0:
            return self._h_spacing
        return self._smart_spacing(QStyle.PixelMetric.PM_LayoutHorizontalSpacing 
                                    if hasattr(self, 'parentWidget') else 6)
    
    def verticalSpacing(self) -> int:
        if self._v_spacing >= 0:
            return self._v_spacing
        return self._smart_spacing(QStyle.PixelMetric.PM_LayoutVerticalSpacing 
                                    if hasattr(self, 'parentWidget') else 6)
    
    def _smart_spacing(self, pm) -> int:
        """Get smart spacing based on parent widget"""
        parent = self.parentWidget()
        if parent is None:
            return 6
        return 6  # Default spacing
    
    def count(self) -> int:
        return len(self._items)
    
    def itemAt(self, index: int) -> QLayoutItem:
        if 0 <= index < len(self._items):
            return self._items[index]
        return None
    
    def takeAt(self, index: int) -> QLayoutItem:
        if 0 <= index < len(self._items):
            return self._items.pop(index)
        return None
    
    def expandingDirections(self) -> Qt.Orientation:
        return Qt.Orientation(0)
    
    def hasHeightForWidth(self) -> bool:
        return True
    
    def heightForWidth(self, width: int) -> int:
        return self._do_layout(QRect(0, 0, width, 0), test_only=True)
    
    def setGeometry(self, rect: QRect):
        super().setGeometry(rect)
        self._do_layout(rect, test_only=False)
    
    def sizeHint(self) -> QSize:
        return self.minimumSize()
    
    def minimumSize(self) -> QSize:
        size = QSize()
        for item in self._items:
            size = size.expandedTo(item.minimumSize())
        
        margins = self.contentsMargins()
        size += QSize(margins.left() + margins.right(),
                      margins.top() + margins.bottom())
        return size
    
    def _do_layout(self, rect: QRect, test_only: bool) -> int:
        """Perform the layout, return the height used"""
        margins = self.contentsMargins()
        effective_rect = rect.adjusted(margins.left(), margins.top(),
                                        -margins.right(), -margins.bottom())
        
        x = effective_rect.x()
        y = effective_rect.y()
        line_height = 0
        h_space = self.horizontalSpacing()
        v_space = self.verticalSpacing()
        
        for item in self._items:
            widget = item.widget()
            if widget is None or not widget.isVisible():
                continue
            
            item_size = item.sizeHint()
            next_x = x + item_size.width() + h_space
            
            # Wrap to next line if needed
            if next_x - h_space > effective_rect.right() and line_height > 0:
                x = effective_rect.x()
                y = y + line_height + v_space
                next_x = x + item_size.width() + h_space
                line_height = 0
            
            if not test_only:
                item.setGeometry(QRect(QPoint(x, y), item_size))
            
            x = next_x
            line_height = max(line_height, item_size.height())
        
        return y + line_height - rect.y() + margins.bottom()
    
    def clear(self):
        """Remove all items from layout"""
        while self._items:
            item = self._items.pop()
            if item.widget():
                item.widget().deleteLater()
