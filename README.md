Hooks
====
*Hooks for trace php functions and class methods*

It supports the following activities:
- Hooking into function execution, add before and after execution callbacks
- Hook function arguments, exceptions and return values

Requirements
============

* PHP 7+

Installation
============

**From sources**

    phpize
    ./configure 
    make
    make test
    make install
Add ```extension=hooks.so``` to your php.ini

API
===
```php
/**
* Inject hook into function
* @param string function
* @param callable before
* @param callable after
**/
function hooks_set_hook(string $function, callable $before, callable $after) : bool;

/**
* Inject hook into class method
* @param string class
* @param string function
* @param callable before
* @param callable after
**/
function hooks_set_hook(string $class, string $method, callable $before, callable $after) : bool;

```

Example
=======
Code:
```php
class Profiler
{
    private float $start;

    public function start($args)
    {
        $this->start = microtime(true);
        echo "start function: {$args['function']}\n";
        echo "args: \n";
        print_r($args['args']);
    }

    public function finish($args)
    {
        $finish = microtime(true);
        echo "end hook: " . ($finish - $this->start) . "ms.\n";
        echo "result: {$args['result']}\n";
        if(isset($args['exception'])){
            /** @var $exception \Throwable */
            $exception = $args['exception'];
            echo "exception message: ".$exception->getMessage()."\n";
        }
    }
}

$profiler = new Profiler();

$start = function ($args) {
/** @var $this Profiler */
$this->start($args);
};

$end = function ($args) {
/** @var $this Profiler */
$this->finish($args);
};

$start_callable = Closure::bind($start, $profiler, 'Profiler');
$end_callable = Closure::bind($end, $profiler, 'Profiler');

hook_set_hook('date', $start_callable, $end_callable);
hook_set_hook('usleep', $start_callable, $end_callable);
hook_set_hook('PDO', '__construct', $start_callable, $end_callable);


echo date('Y.m.d')."\n";
usleep(1000);

try{
    $pdo = new PDO('mysql:dbname=error;host=error');
}catch(\Throwable $e){
    echo $e->getMessage()."\n";
}
```

Output:
```
start function: date
args:
Array
(
    [0] => Y.m.d
)
end hook: 0.0025720596313477ms.
result: 2022.04.14
2022.04.14


start function: usleep
args:
Array
(
    [0] => 1000
)
end hook: 0.0011129379272461ms.
result:


start function: __construct
args:
Array
(
    [0] => mysql:dbname=error;host=error
)
end hook: 0.00052905082702637ms.
result:
exception message: could not find driver
could not find driver
```