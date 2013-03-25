/*
 * mapi.h
 *
 *  Created on: 30.04.2011
 *      Author: igor
 */

#ifndef MAPI_H_
#define MAPI_H_

#include <map>
#include <vector>
#include <iostream>
#include <cassert>
#include <boost/thread/mutex.hpp>

// опережающее описание mapi
template <typename Key, typename T>
class mapi;
/*
 * Вспомогательный класс reference_mapped_type. Фактически в mapi::m_map вместо T хранится он.
 * Реализует изменение индекса mapi::m_index в случае измении mapi::m_map с помощью итераторов и индексации.
 * Пример:
 * mapi<string, int> a;
 * a.insert(make_pair("test1",1));
 * mapi<string, int>::iterator i = a.find("test1");
 * i->second = 2;
 * (*i).second = 3;
 * a["test1"] = 4;
 *
 * Имеет конструктор по умолчанию, который не должен использоваться, reference_mapped_type должен
 * всегда создаваться классом mapi при помощи private конструктора. Добавлен только для того, что-бы была
 * возможна индексация (operator[]), это требует класс std::map.
 *
 * Для доступа к значению T реализован оператор приведения типа operator T() и оператор присваивания
 * reference_mapped_type& operator=(const T&). Для операций ++, --, +=  и т.п. этого недостаточно.
 *
 */
template <typename Keyr, typename Tr>
class reference_mapped_type {
	template <typename Key, typename T>
	friend class mapi;
public:
	//оператор приведения типа
	operator Tr() const { return m_value; }
	//оператор присваивания
	reference_mapped_type& operator=(const Tr& v) {
		assert(m_mapi); //в случае неправильного использования возникает assert
		boost::mutex::scoped_lock lock(m_mapi->m_mutex);
		if (v != m_value) {
			m_mapi->delindex(m_mapi->m_map.find(m_key));
			m_mapi->m_index.insert(make_pair(v, m_key));
			m_value = v;
		}
		return *this;
	}
	reference_mapped_type& operator=(const reference_mapped_type& m) {
		assert(m_mapi);
		boost::mutex::scoped_lock lock(m_mapi->m_mutex);
		if (m.m_value != m_value) {
			m_mapi->delindex(m_mapi->m_map.find(m_key));
			m_mapi->m_index.insert(make_pair(m.m_value, m_key));
			m_value = m.m_value;
		}
		return *this;
	}
	bool operator== (const Tr& v) const { return m_value == v; }
	bool operator!= (const Tr& v) const { return m_value != v; }
	bool operator< (const Tr& v) const { return m_value < v; }
	reference_mapped_type() : m_mapi(0), m_key(Keyr()), m_value(Tr()) {}
private:
	reference_mapped_type(mapi<Keyr, Tr> *m, const Keyr& k, const Tr& v) : m_mapi(m), m_key(k), m_value(v) {}
	//указатель на родительский объект
	mapi<Keyr, Tr> *m_mapi;
	//ключ, которому в mapi:m_map соответствует m_value
	Keyr m_key;
	//собственно значение
	Tr m_value;
};
/*
 * Класс mapi. Представляет из себя урезанный вариант класса map, но с возможность быстрого поиска по значению.
 * В частности урезаны параметры шаблона: тип объекта сравнения Compare, аллокатор Alloc. В качестве их типов
 * приняты значения по умолчанию.
 *
 * Хранение данных осуществляется в std::map m_map. Индекс быстрого поиска хранится в std::multimap m_index.
 * Для быстрого поиска по значению реализован метод std::vector<iterator> findv(const T&) и константный
 * вариант std::vector<const_iterator> findv(const T&) const, которые возвращают вектор итераторов.
 *
 * Реализована потокобезопастность методов добавления, удаления и поиска. Потокобезопастность реализована
 * с помощью класса boost::mutex
 *
 * Требования к классам Key и T теже, что и к соотвествующим классам std::map. Дополнительное требование
 * к классу T иметь операции ==, != и <.
 *
 */
template <typename Key, typename T>
class mapi {
	template <typename Keyr, typename Tr>
	friend class reference_mapped_type;
	template <typename CharT, typename Tratis, typename Keyf, typename Tf>
	friend std::basic_ostream<CharT, Tratis>& operator<< (std::basic_ostream<CharT, Tratis>&, const mapi<Keyf, Tf>&);
public:
	//тип ключа
	typedef Key key_type;
	//тип хранимых данные
	typedef reference_mapped_type<Key, T> mapped_type;
	//тип основного хранилища
	typedef std::map<key_type, mapped_type> map;
	//тип индекса
	typedef std::multimap<T, Key> index_type;
	//тип основного итератора
	typedef typename map::iterator iterator;
	//тип основного константного итератора
	typedef typename map::const_iterator const_iterator;
	//тип размера
	typedef typename map::size_type size_type;
	//тип индексного итератора
	typedef typename index_type::iterator index_iterator;
	//тип констатного индексного итератора
	typedef typename index_type::const_iterator index_const_iterator;
	//тип пара индексных итераторов
	typedef std::pair<index_iterator, index_iterator> pair_index_iterator;
	//тип пара констатных индексных итераторов
	typedef std::pair<index_const_iterator, index_const_iterator> pair_index_const_iterator;
	//тип пары основного хранилища
	typedef std::pair<const key_type, mapped_type> value_type;
	//конструктор по умолчанию
	mapi() { }
	//копирующий конструктор из std::map
	mapi(const std::map<Key, T>& x) {
		for (typename std::map<Key, T>::const_iterator i = x.begin(); i != x.end(); ++i) {
			m_map.insert(make_pair(i->first, reference_mapped_type<Key, T>(this, i->first, i->second)));
			m_index.insert(make_pair(i->second, i->first));
		}
	}
	//копирующий конструктор из mapi
	mapi(const mapi& x) {
		for (const_iterator i = x.begin(); i != x.end(); ++i) {
			m_map.insert(make_pair(i->first, reference_mapped_type<Key, T>(this, i->first, i->second)));
			m_index.insert(make_pair(i->second, i->first));
		}
	}
	//конструктор из диапазона итераторов
	template<typename InputIterator>
	mapi(InputIterator first, InputIterator last) {
		for (; first != last; ++first) {
			std::pair<iterator, bool> pair_ib = m_map.insert(make_pair(first->first, reference_mapped_type<Key, T>(this, first->first, first->second)));
			if (pair_ib.second)
				m_index.insert(make_pair(first->second, first->first));
		}
	}
	//оператор копирования из std::map
	mapi& operator=(const std::map<Key, T>& x) {
		boost::mutex::scoped_lock lock(m_mutex);
		m_map.clear();
		m_index.clear();
		for (typename std::map<Key, T>::const_iterator i = x.begin(); i != x.end(); ++i) {
			m_map.insert(make_pair(i->first, reference_mapped_type<Key, T>(this, i->first, i->second)));
			m_index.insert(make_pair(i->second, i->first));
		}
		return *this;
	}
	//оператор копирования из mapi
	mapi& operator=(const mapi& x) {
		boost::mutex::scoped_lock lock(m_mutex);
		if (this != &x) {
			m_map.clear();
			m_index.clear();
			for (const_iterator i = x.begin(); i != x.end(); ++i) {
				m_map.insert(make_pair(i->first, reference_mapped_type<Key, T>(this, i->first, i->second)));
				m_index.insert(make_pair(i->second, i->first));
			}
		}
		return *this;
	}
	//вставка значения
	std::pair<iterator, bool> insert(const std::pair<Key, T>& x) {
		boost::mutex::scoped_lock lock(m_mutex);
		std::pair<iterator, bool> pair_ib = m_map.insert(make_pair(x.first, reference_mapped_type<Key, T>(this, x.first, x.second)));
		if (pair_ib.second)
			m_index.insert(std::make_pair(x.second, x.first));
		return pair_ib;
	}
	//вставка значения с указанием подсказывающего (hint) итератора
	iterator insert(iterator position, const std::pair<Key, T>& x)	{
		boost::mutex::scoped_lock lock(m_mutex);
		if (m_map.find(x.first) == m_map.end())
			m_index.insert(std::make_pair(x.second, x.first));
		return m_map.insert(position, make_pair(x.first, reference_mapped_type<Key, T>(this, x.first, x.second)));
	}
	//вставка из диапазона итераторов
	template<typename InputIterator>
	void insert(InputIterator first, InputIterator last) {
		boost::mutex::scoped_lock lock(m_mutex);
		for (; first != last; ++first) {
			std::pair<iterator, bool> pair_ib = m_map.insert(make_pair(first->first, reference_mapped_type<Key, T>(this, first->first, first->second)));
			if (pair_ib.second)
				m_index.insert(make_pair(first->second, first->first));
		}
	}
	//итератор начала
	iterator begin() {
		return m_map.begin();
	}
	//константный итератор начала
	const_iterator 	begin() const {
		return m_map.begin();
	}
	//итератор конца
	iterator end() {
		return m_map.end();
	}
	//константный итератор конца
	const_iterator end() const {
		return m_map.end();
	}
	//удаление по итератору
	void erase(iterator position) {
		boost::mutex::scoped_lock lock(m_mutex);
		delindex(position);
		m_map.erase(position);
	}
	//удаление по значению
	size_type erase(const Key& x) {
		boost::mutex::scoped_lock lock(m_mutex);
		iterator i = m_map.find(x);
		delindex(i);
		return m_map.erase(x);
	}
	//удаление по диапазону итераторов
	void erase(iterator first, iterator last) {
		boost::mutex::scoped_lock lock(m_mutex);
		for (iterator i = first; i != last; ++i)
			delindex(i);
		m_map.erase(first, last);
	}
	//поиск по значению
	std::vector<iterator> findv(const T& v) {
		boost::mutex::scoped_lock lock(m_mutex);
		pair_index_iterator pairi = m_index.equal_range(v);
		std::vector<iterator> vec;
		for (; pairi.first != pairi.second; ++pairi.first)
			vec.push_back(m_map.find(pairi.first->second));
		return vec;
	}
	//константный поиск по значению
	std::vector<const_iterator> findv(const T& v) const {
		boost::mutex::scoped_lock lock(m_mutex);
		pair_index_const_iterator pairi = m_index.equal_range(v);
		std::vector<const_iterator> vec;
		for (; pairi.first != pairi.second; ++pairi.first)
			vec.push_back(m_map.find(pairi.first->second));
		return vec;
	}
	//поиск по ключу
	iterator find(const key_type& x) {
		boost::mutex::scoped_lock lock(m_mutex);
		return m_map.find(x);
	}
	//константный поиск по ключу
	const_iterator find(const key_type& x) const {
		boost::mutex::scoped_lock lock(m_mutex);
		return m_map.find(x);
	}
	//операция индексации
	mapped_type& operator[](const key_type& k) {
		boost::mutex::scoped_lock lock(m_mutex);
		if (m_map.find(k) == m_map.end()) {
			m_map.insert(make_pair(k, reference_mapped_type<Key, T>(this, k, T())));
			m_index.insert(make_pair(T(), k));
		}
		return m_map[k];
	}
	//очистка
	void clear() {
		boost::mutex::scoped_lock lock(m_mutex);
		m_map.clear();
		m_index.clear();
	}
	//проверка на пустоту
	bool empty() const {
		return m_map.empty();
	}
	//размер
	size_type size() const {
		return m_map.size();
	}
	//проверка индекса на корректность, для тестирования
	bool validate() const {
		boost::mutex::scoped_lock lock(m_mutex);
		if (m_map.size() != m_index.size())
			return false;
		for (const_iterator i = m_map.begin(); i != m_map.end(); ++i) {
			pair_index_const_iterator pairci = m_index.equal_range(i->second);
			int count = 0;
			for (index_const_iterator j = pairci.first; j != pairci.second; ++j) {
				if (j->second == i->first)
					++count;
			}
			if (count != 1)
				return false;
		}
		return true;
	}
private:
	//основное хранилище
	map m_map;
	//индекс
	index_type m_index;
	mutable boost::mutex m_mutex;
	//вспомогательный метод удаления из индекса по итератору основного хранилища
	void delindex(iterator i) {
		if (i == m_map.end())
			return;
		pair_index_iterator pairi = m_index.equal_range(i->second);
		bool found = false;
		for (; pairi.first != pairi.second; ++pairi.first)
			if (pairi.first->second == i->first) {
				found = true;
				break;
			}
		assert(found); // срабатывание, означает ошибку в программе
		m_index.erase(pairi.first);
	}
};

template <typename CharT, typename Tratis, typename Keyf, typename Tf>
std::basic_ostream<CharT, Tratis>& operator<< (std::basic_ostream<CharT, Tratis>& os, const mapi<Keyf, Tf>& x) {
	os << "\nMap:\n";
	for (typename mapi<Keyf, Tf>::const_iterator i = x.m_map.begin(); i != x.m_map.end(); ++i)
		os << i->first << '\t' << i->second << '\n';
	os << "\nIndex:\n";
	for (typename mapi<Keyf, Tf>::index_const_iterator i = x.m_index.begin(); i != x.m_index.end(); ++i)
		os << i->first << '\t' << i->second << '\n';
	return os;
}

#endif /* MAPI_H_ */
