# openvpn-api-rest
Restful API for OPENVPN

Данное приложение парсит файл openvpn-status.log и отдает данные о подключенных пользователях к OpenVPN в формате json при обращении к адресу /api/openvpn/connected
Приложение запускается с аргументом, который указывает на путь до файла openvpn-status.log
````
make
./api /var/log/openvpn-status.log
````
