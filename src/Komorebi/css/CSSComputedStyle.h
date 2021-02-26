#ifndef COCOA_CSSCOMPUTEDSTYLE_H
#define COCOA_CSSCOMPUTEDSTYLE_H

#include <memory>

#include "Komorebi/Komorebi.h"

struct css_select_results;
KMR_NAMESPACE_BEGIN

class CSSComputedStyle
{
public:
    /**
     * @note These properties are computed. They do not correspond
     *       to the list of properties in the CSS standard.
     */
    enum class Property
    {
        kPropLetterSpacing,
        kPropOutlineColor,
        kPropOutlineWidth,
        kPropBorderSpacing,
        kPropWordSpacing,
        kPropCounterIncrement,
        kPropCounterReset,
        kPropCursor,
        kPropClip,
        kPropContent,
        kPropVerticalAlign,
        kPropFontSize,
        kPropBorderTopWidth,
        kPropBorderRightWidth,
        kPropBorderBottomWidth,
        kPropBorderLeftWidth,
        kPropBackgroundImage,
        kPropColor,
        kPropListStyleImage,
        kPropQuotes,
        kPropTop,
        kPropRight,
        kPropBottom,
        kPropLeft,
        kPropBorderTopColor,
        kPropBorderRightColor,
        kPropBorderBottomColor,
        kPropBorderLeftColor,
        kPropBoxSizing,
        kPropHeight,
        kPropLineHeight,
        kPropBackgroundColor,
        kPropZIndex,
        kPropMarginTop,
        kPropMarginRight,
        kPropMarginBottom,
        kPropMarginLeft,
        kPropBackgroundAttachment,
        kPropBorderCollapse,
        kPropCaptionSide,
        kPropDirection,
        kPropMaxHeight,
        kPropMaxWidth,
        kPropWidth,
        kPropEmptyCells,
        kPropFloat,
        kPropWritingMode,
        kPropFontStyle,
        kPropMinHeight,
        kPropMinWidth,
        kPropBackgroundRepeat,
        kPropClear,
        kPropPaddingTop,
        kPropPaddingRight,
        kPropPaddingBottom,
        kPropPaddingLeft,
        kPropOverflowX,
        kPropOverflowY,
        kPropPosition,
        kPropOpacity,
        kPropTextTransform,
        kPropTextIndent,
        kPropWhiteSpace,
        kPropBackgroundPosition,
        kPropBreakAfter,
        kPropBreakBefore,
        kPropBreakInside,
        kPropColumnCount,
        kPropColumnFill,
        kPropColumnGap,
        kPropColumnRuleColor,
        kPropColumnRuleStyle,
        kPropColumnRuleWidth,
        kPropColumnSpan,
        kPropColumnWidth,
        kPropDisplay,
        kPropDisplayStatic,
        kPropFontVariant,
        kPropTextDecoration,
        kPropFontFamily,
        kPropBorderTopStyle,
        kPropBorderRightStyle,
        kPropBorderBottomStyle,
        kPropBorderLeftStyle,
        kPropFontWeight,
        kPropListStyleType,
        kPropOutlineStyle,
        kPropTableLayout,
        kPropUnicodeBidi,
        kPropVisibility,
        kPropListStylePosition,
        kPropTextAlign,
        kPropPageBreakAfter,
        kPropPageBreakBefore,
        kPropPageBreakInside,
        kPropOrphans,
        kPropWidows,
        kPropAlignContent,
        kPropAlignItems,
        kPropAlignSelf,
        kPropFlexBasis,
        kPropFlexDirection,
        kPropFlexGrow,
        kPropFlexShrink,
        kPropFlexWrap,
        kPropJustifyContent,
        kPropOrder
    };

    enum class PVAlignContent
    {
        kInherit,
        kStretch,
        kFlexStart,
        kFlexEnd,
        kCenter,
        kSpaceBetween,
        kSpaceAround,
        kSpaceEvenly
    };

    enum class PVAlignItems
    {
        kInherit,
        kStretch,
        kFlexStart,
        kFlexEnd,
        kCenter,
        kBaseline
    };

    enum class PVAlignSelf
    {
        kInherit,
        kStretch,
        kFlexStart,
        kFlexEnd,
        kCenter,
        kBaseline,
        kAuto
    };

    enum class PVBackgroundAttachment
    {
        kInherit,
        kFixed,
        kScroll
    };

    enum class PVBackgroundColor
    {
        kInherit,
        kColor,
        kCurrentColor
    };

    enum class PVBackgroundImage
    {
        kInherit,
        kNone,
        kImage
    };

    enum class PVBackgroundPosition
    {
        kInherit,
        kSet
    };

    enum class PVBackgroundRepeat
    {
        kInherit,
        kRepeatX,
        kRepeatY,
        kRepeat,
        kNoRepeat
    };

    enum class PVBorderCollapse
    {
        kInherit,
        kSeparate,
        kCollapse
    };

    enum class PVBorderSpacing
    {
        kInherit,
        kSet
    };

    enum class PVBorderColor
    {
        kInherit,
        kColor,
        kCurrentColor
    };

    enum class PVBorderStyle
    {
        kInherit,
        kNone,
        kHidden,
        kDotted,
        kDashed,
        kSolid,
        kDouble,
        kGroove,
        kRidge,
        kInset,
        kOutset
    };

    enum class PVBorderWidth
    {
        kInherit,
        kThin,
        kMedium,
        kThick,
        kWidth
    };

    enum class PVBottom
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVBoxSizing
    {
        kInherit,
        kContentBox,
        kBorderBox
    };

    enum class PVBreakAfter
    {
        kInherit,
        kAuto,
        kAvoid,
        kAlways,
        kLeft,
        kRight,
        kPage,
        kColumn,
        kAvoidPage,
        kAvoidColumn
    };

    enum class PVBreakBefore
    {
        kInherit,
        kAuto,
        kAvoid,
        kAlways,
        kLeft,
        kRight,
        kPage,
        kColumn,
        kAvoidPage,
        kAvoidColumn
    };

    enum class PVBreakInside
    {
        kInherit,
        kAuto,
        kAvoid,
        kAvoidPage,
        kAvoidColumn
    };

    enum class PVCaptionSide
    {
        kInherit,
        kTop,
        kBottom
    };

    enum class PVClear
    {
        kInherit,
        kNone,
        kLeft,
        kRight,
        kBoth
    };

    enum class PVClip
    {
        kInherit,
        kAuto,
        kRect
    };

    enum class PVColor
    {
        kInherit,
        kColor
    };

    enum class PVColumnCount
    {
        kInherit,
        kAuto,
        kSet
    };

    enum class PVColumnFill
    {
        kInherit,
        kBalance,
        kAuto
    };

    enum class PVColumnGap
    {
        kInherit,
        kSet,
        kNormal
    };

    enum class PVColumnRuleColor
    {
        kInherit,
        kColor,
        kCurrentColor
    };

    enum class PVColumnRuleStyle
    {
        kInherit,
        kNone,
        kHidden,
        kDotted,
        kDashed,
        kSolid,
        kDouble,
        kGroove,
        kRidge,
        kInset,
        kOutset
    };

    enum class PVColumnRuleWidth
    {
        kInherit,
        kThin,
        kMedium,
        kThick,
        kWidth
    };

    enum class PVColumnSpan
    {
        kInherit,
        kNone,
        kAll
    };

    enum class PVColumnWidth
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVContent
    {
        kInherit,
        kNone,
        kNormal,
        kSet
    };

    enum class PVCounterIncrement
    {
        kInherit,
        kNamed,
        kNone
    };

    enum class PVCounterReset
    {
        kInherit,
        kNamed,
        kNone
    };

    enum class PVCursor
    {
        kInherit,
        kAuto,
        kCrosshair,
        kDefault,
        kPointer,
        kMove,
        kEResize,
        kNeResize,
        kNwResize,
        kNResize,
        kSeResize,
        kSwResize,
        kSResize,
        kWResize,
        kText,
        kWait,
        kHelp,
        kProgress
    };

    enum class PVDirection
    {
        kInherit,
        kLtr,
        kRtl
    };

    enum class PVDisplay
    {
        kInherit,
        kInline,
        kBlock,
        kListItem,
        kRunIn,
        kInlineBlock,
        kTable,
        kInlineTable,
        kTableRowGroup,
        kTableHeaderGroup,
        kTableFooterGroup,
        kTableRow,
        kTableColumnGroup,
        kTableColumn,
        kTableCell,
        kTableCaption,
        kNone,
        kFlex,
        kInlineFlex
    };

    enum class PVEmptyCells
    {
        kInherit,
        kShow,
        kHide
    };

    enum class PVFlexBasis
    {
        kInherit,
        kSet,
        kAuto,
        kContent
    };

    enum class PVFlexDirection
    {
        kInherit,
        kRow,
        kRowReverse,
        kColumn,
        kColumnReverse
    };

    enum class PVFlexGrow
    {
        kInherit,
        kSet
    };

    enum class PVFlexShrink
    {
        kInherit,
        kSet
    };

    enum class PVFlexWrap
    {
        kInherit,
        kNowrap,
        kWrap,
        kWrapReverse
    };

    enum class PVFloat
    {
        kInherit,
        kLeft,
        kRight,
        kNone
    };

    enum class PVFontFamily
    {
        kInherit,
        kSerif,
        kSansSerif,
        kCursive,
        kFantasy,
        kMonospace
    };

    enum class PVFontSize
    {
        kInherit,
        kXxSmall,
        kXSmall,
        kSmall,
        kMedium,
        kLarge,
        kXLarge,
        kXxLarge,
        kLarger,
        kSmaller,
        kDimension
    };

    enum class PVFontStyle
    {
        kInherit,
        kNormal,
        kItalic,
        kOblique
    };

    enum class PVFontVariant
    {
        kInherit,
        kNormal,
        kSmallCaps
    };

    enum class PVFontWeight
    {
        kInherit,
        kNormal,
        kBold,
        kBolder,
        kLighter,
        k_100,
        k_200,
        k_300,
        k_400,
        k_500,
        k_600,
        k_700,
        k_800,
        k_900
    };

    enum class PVHeight
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVJustifyContent
    {
        kInherit,
        kFlexStart,
        kFlexEnd,
        kCenter,
        kSpaceBetween,
        kSpaceAround,
        kSpaceEvenly
    };

    enum class PVLeft
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVLetterSpacing
    {
        kInherit,
        kSet,
        kNormal
    };

    enum class PVLineHeight
    {
        kInherit,
        kNumber,
        kDimension,
        kNormal
    };

    enum class PVListStyleImage
    {
        kInherit,
        kUri,
        kNone
    };

    enum class PVListStylePosition
    {
        kInherit,
        kInside,
        kOutside
    };

    enum class PVListStyleType
    {
        kInherit,
        kDisc,
        kCircle,
        kSquare,
        kDecimal,
        kDecimalLeadingZero,
        kLowerRoman,
        kUpperRoman,
        kLowerGreek,
        kLowerLatin,
        kUpperLatin,
        kArmenian,
        kGeorgian,
        kLowerAlpha,
        kUpperAlpha,
        kNone
    };

    enum class PVMargin
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVMaxHeight
    {
        kInherit,
        kSet,
        kNone
    };

    enum class PVMaxWidth
    {
        kInherit,
        kSet,
        kNone
    };

    enum class PVMinHeight
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVMinWidth
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVOpacity
    {
        kInherit,
        kSet
    };

    enum class PVOrder
    {
        kInherit,
        kSet
    };

    enum class PVOutlineColor
    {
        kInherit,
        kColor,
        kCurrentColor,
        kInvert
    };

    enum class PVOutlineStyle
    {
        kInherit,
        kNone,
        kDotted,
        kDashed,
        kSolid,
        kDouble,
        kGroove,
        kRidge,
        kInset,
        kOutset
    };

    enum class PVOutlineWidth
    {
        kInherit,
        kThin,
        kMedium,
        kThick,
        kWidth
    };

    enum class PVOverflow
    {
        kInherit,
        kVisible,
        kHidden,
        kScroll,
        kAuto
    };

    enum class PVOrphans
    {
        kInherit,
        kSet
    };

    enum class PVPadding
    {
        kInherit,
        kSet
    };

    enum class PVPageBreakAfter
    {
        kInherit,
        kAuto,
        kAvoid,
        kAlways,
        kLeft,
        kRight
    };

    enum class PVPageBreakBefore
    {
        kInherit,
        kAuto,
        kAvoid,
        kAlways,
        kLeft,
        kRight
    };

    enum class PVPageBreakInside
    {
        kInherit,
        kAuto,
        kAvoid
    };

    enum class PVPosition
    {
        kInherit,
        kStatic,
        kRelative,
        kAbsolute,
        kFixed
    };

    enum class PVQuotes
    {
        kInherit,
        kString,
        kNone
    };

    enum class PVRight
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVTableLayout
    {
        kInherit,
        kAuto,
        kFixed
    };

    enum class PVTextAlign
    {
        kInherit
    };

    enum class PVTextDecoration
    {
        kInherit,
        kNone,
        kBlink,
        kLineThrough,
        kOverline,
        kUnderline
    };

    enum class PVTextIndent
    {
        kInherit,
        kSet
    };

    enum class PVTextTransform
    {
        kInherit,
        kCapitalize,
        kUppercase,
        kLowercase,
        kNone
    };

    enum class PVTop
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVUnicodeBidi
    {
        kInherit,
        kNormal,
        kEmbed,
        kBidiOverride
    };

    enum class PVVerticalAlign
    {
        kInherit,
        kBaseline,
        kSub,
        kSuper,
        kTop,
        kTextTop,
        kMiddle,
        kBottom,
        kTextBottom,
        kSet
    };

    enum class PVVisibility
    {
        kInherit,
        kVisible,
        kHidden,
        kCollapse
    };

    enum class PVWhiteSpace
    {
        kInherit,
        kNormal,
        kPre,
        kNowrap,
        kPreWrap,
        kPreLine
    };

    enum class PVWidows
    {
        kInherit,
        kSet
    };

    enum class PVWidth
    {
        kInherit,
        kSet,
        kAuto
    };

    enum class PVWordSpacing
    {
        kInherit,
        kSet,
        kNormal
    };

    enum class PVWritingMode
    {
        kInherit,
        kHorizontalTb,
        kVerticalRl,
        kVerticalLr
    };

    enum class PVZIndex
    {
        kInherit,
        kSet,
        kAuto
    };

    explicit CSSComputedStyle(css_select_results *pResults);
    CSSComputedStyle(CSSComputedStyle&& other) noexcept ;
    CSSComputedStyle(const CSSComputedStyle&) = delete;
    CSSComputedStyle& operator=(const CSSComputedStyle&) = delete;

    ~CSSComputedStyle();

    css_select_results *nativeHandle() noexcept;

private:
    css_select_results      *fResults;
};

KMR_NAMESPACE_END
#endif //COCOA_CSSCOMPUTEDSTYLE_H
