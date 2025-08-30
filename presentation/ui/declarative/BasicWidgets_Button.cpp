#include "BasicWidgets_Button.h"

namespace UI {

std::unique_ptr<IUiComponent> Button::build() const {
	// 创建运行时按钮组件
	auto pushButton = std::make_unique<UiPushButton>();
	
	// 配置基本属性
	pushButton->setText(m_text);
	
	// 配置变体（转换枚举）
	switch (m_variant) {
		case Variant::Primary:
			pushButton->setVariant(UiPushButton::Variant::Primary);
			break;
		case Variant::Secondary:
			pushButton->setVariant(UiPushButton::Variant::Secondary);
			break;
		case Variant::Ghost:
			pushButton->setVariant(UiPushButton::Variant::Ghost);
			break;
	}
	
	// 配置尺寸（转换枚举）
	switch (m_size) {
		case Size::S:
			pushButton->setSize(UiPushButton::Size::S);
			break;
		case Size::M:
			pushButton->setSize(UiPushButton::Size::M);
			break;
		case Size::L:
			pushButton->setSize(UiPushButton::Size::L);
			break;
	}
	
	// 配置图标
	if (m_useThemeIcons) {
		pushButton->setIconThemePaths(m_iconLightPath, m_iconDarkPath);
	} else if (!m_iconPath.isEmpty()) {
		pushButton->setIconPath(m_iconPath);
	}
	
	// 配置其他属性
	pushButton->setCornerRadius(m_cornerRadius);
	pushButton->setDisabled(m_disabled);
	
	if (m_useCustomPadding) {
		pushButton->setPadding(m_padding);
	}
	
	// 配置回调
	if (m_onTap) {
		pushButton->setOnTap(m_onTap);
	}
	
	// 应用装饰器并返回
	return decorate(std::move(pushButton));
}

} // namespace UI