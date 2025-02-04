# Поисковой сервер

## Описание
**Поисковой сервер** - это инструмент на базе C++, предназначенный для нахождения наиболее релевантных документов по текстовому запросу. Данный инструмент обрабатывает переданные ему стоп-слова и текстовые документы, разбивает текст на слова и подсчитывает их частотность, храня эти данные внутри себя. При получении запроса инструмент разбивает его на плюс и минус слова, а затем расчитывает IDF (Inverse Document Frequency) и TF (Term Frequency) для каждого из плюс слов. Как итог все документы с совпадениями сортируются и в результате выдаются первые 5 документов с наибольшей релевантностью.

## Возможности
* Имеется возможность добавления и удаления документов в **поисковом сервере** при помощи методов *AddDocument(...)* и *RemoveDocument(...)*.
* При помощи функции-предиката можно произвести сортировку результатов по статусу, id и рейтингу документа, передавая ее дополнительным параметром в метод *FindTopDocuments(...)*.
* Имеется возможность разбивать результаты поиска по страницам, используя функцию *Peginate(...)*.
* Имеется возможность поиска совпадений слов из запроса в документе при помощи метода *MatchDocument(...)*. В случае обнаружения минус слова в документе, все обнаруженные совпадения перестают учитываться.
* Имеется возможность получить частотность слов в документе при помощи метода *GetWordFrequencies(...)*.
* Имеется дедупликатор документов, который исключает из **поискового сервера** копии документов. Данная операция осуществляется при помощи функции *RemoveDuplicates(...)*.
* Присутствует поддержка мультипоточности.

## Требования
Для установки **поискового сервера** требуется система сборки проектов CMake, версии не ниже 3.11.

## Установка
1) Создать паку build в каталоге с файлом *CmakeLists.txt* и перейти в нее
2) Открыть консоль в данной папке
3) Ввести в консоль комманды:

```
cmake .. -G "MinGW Makefiles"
cmake --build .
```
4) Готово! **Поисковой сервер** установлен на ваше устройство.