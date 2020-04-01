#pragma once

namespace Utils
{
	template <typename T>
	class concurrent_list final
	{
	public:
		class element final
		{
		public:
			explicit element(std::recursive_mutex* mutex, std::shared_ptr<T> entry = {},
			                 std::shared_ptr<element> next = {}) :
				mutex_(mutex),
				entry_(std::move(entry)),
				next_(std::move(next))
			{
			}

			void remove(const std::shared_ptr<T>& element)
			{
				std::lock_guard _(*this->mutex_);
				if (!this->next_) return;

				if (this->next_->entry_.get() == element.get())
				{
					this->next_ = this->next_->next_;
				}
				else
				{
					this->next_->remove(element);
				}
			}

			std::shared_ptr<element> get_next() const
			{
				std::lock_guard _(*this->mutex_);
				return this->next_;
			}

			std::shared_ptr<T> operator*() const
			{
				std::lock_guard _(*this->mutex_);
				return this->entry_;
			}

			element& operator++()
			{
				std::lock_guard _(*this->mutex_);
				*this = this->next_ ? *this->next_ : element(this->mutex_);
				return *this;
			}

			element operator++(int)
			{
				std::lock_guard _(*this->mutex_);
				auto result = *this;
				this->operator++();
				return result;
			}

			bool operator==(const element& other)
			{
				std::lock_guard _(*this->mutex_);
				return this->entry_.get() == other.entry_.get();
			}

			bool operator!=(const element& other)
			{
				std::lock_guard _(*this->mutex_);
				return !(*this == other);
			}

		private:
			std::recursive_mutex* mutex_;
			std::shared_ptr<T> entry_;
			std::shared_ptr<element> next_;
		};

		element begin()
		{
			std::lock_guard _(this->mutex_);
			return this->entry_ ? *this->entry_ : this->end();
		}

		element end()
		{
			std::lock_guard _(this->mutex_);
			return element(&this->mutex_);
		}

		void remove(const element& entry)
		{
			std::lock_guard _(this->mutex_);
			this->remove(*entry);
		}

		void remove(const std::shared_ptr<T>& element)
		{
			std::lock_guard _(this->mutex_);
			if (!this->entry_) return;

			if ((**this->entry_).get() == element.get())
			{
				this->entry_ = this->entry_->get_next();
			}
			else
			{
				this->entry_->remove(element);
			}
		}

		void add(const T& object)
		{
			std::lock_guard _(this->mutex_);

			const auto object_ptr = std::make_shared<T>(object);
			this->entry_ = std::make_shared<element>(&this->mutex_, object_ptr, this->entry_);
		}

		void clear()
		{
			std::lock_guard _(this->mutex_);
			this->entry_ = {};
		}

	private:
		std::recursive_mutex mutex_;
		std::shared_ptr<element> entry_;
	};
}
