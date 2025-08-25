#pragma once
#include "UiBoxLayout.h"
#include "Widget.h"

namespace UI {

	// 布局对齐方式
	enum class Alignment {
		Start,
		Center,
		End,
		Stretch,
		SpaceBetween,
		SpaceAround,
		SpaceEvenly
	};

	// Column - 垂直布局
	class Column : public Widget {
	public:
		Column(WidgetList children = {}) : m_children(std::move(children)) {}

		// 链式调用
		std::shared_ptr<Column> spacing(int s) {
			m_spacing = s;
			return self<Column>();
		}

		std::shared_ptr<Column> mainAxisAlignment(Alignment align) {
			m_mainAxisAlignment = align;
			return self<Column>();
		}

		std::shared_ptr<Column> crossAxisAlignment(Alignment align) {
			m_crossAxisAlignment = align;
			return self<Column>();
		}

		std::shared_ptr<Column> children(WidgetList children) {
			m_children = std::move(children);
			return self<Column>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetList m_children;
		int m_spacing{ 0 };
		Alignment m_mainAxisAlignment{ Alignment::Start };
		Alignment m_crossAxisAlignment{ Alignment::Start };
	};

	// Row - 水平布局
	class Row : public Widget {
	public:
		Row(WidgetList children = {}) : m_children(std::move(children)) {}

		std::shared_ptr<Row> spacing(int s) {
			m_spacing = s;
			return self<Row>();
		}

		std::shared_ptr<Row> mainAxisAlignment(Alignment align) {
			m_mainAxisAlignment = align;
			return self<Row>();
		}

		std::shared_ptr<Row> crossAxisAlignment(Alignment align) {
			m_crossAxisAlignment = align;
			return self<Row>();
		}

		std::shared_ptr<Row> children(WidgetList children) {
			m_children = std::move(children);
			return self<Row>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetList m_children;
		int m_spacing{ 0 };
		Alignment m_mainAxisAlignment{ Alignment::Start };
		Alignment m_crossAxisAlignment{ Alignment::Start };
	};

	// Stack - 层叠布局
	class Stack : public Widget {
	public:
		Stack(WidgetList children = {}) : m_children(std::move(children)) {}

		std::shared_ptr<Stack> alignment(Alignment align) {
			m_alignment = align;
			return self<Stack>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetList m_children;
		Alignment m_alignment{ Alignment::Center };
	};

	// Expanded - 弹性布局子项
	class Expanded : public Widget {
	public:
		Expanded(WidgetPtr child, float flex = 1.0f)
			: m_child(std::move(child)), m_flex(flex) {
		}

		std::unique_ptr<IUiComponent> build() const override {
			return m_child->build();
		}

		float flex() const { return m_flex; }

	private:
		WidgetPtr m_child;
		float m_flex;
	};

	// Spacer - 空白间隔
	class Spacer : public Widget {
	public:
		explicit Spacer(int size = 0) : m_size(size) {}
		std::unique_ptr<IUiComponent> build() const override;

	private:
		int m_size;
	};

} // namespace UI