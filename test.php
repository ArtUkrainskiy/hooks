<?
$before = function ($args) {
    echo "before function called\n";
    print_r($args);
};

$after = function ($args) {
    echo "after function called\n";
    print_r($args);
};

hooks_set_hook('PDO', '__construct', $before, $after);


try {
    // Подключение к базе данных SQLite
    $pdo = new PDO('sqlite:/database.db');
} catch (Exception $e) {
    echo 'Connection failed: ' . $e->getMessage();
};

