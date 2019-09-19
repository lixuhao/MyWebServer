/*
 *@Athor liixuhao
 *@Email xxxqq.com
 */

#pragma once


class noncopyable
{
protected:
  noncopyable() {};
  ~noncopyable() {};
private:
  //拷贝构造函数和赋值运算符重载函数都是不能被拷贝的。
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};
